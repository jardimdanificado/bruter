#include "bruter.h"

// Função para criar um processo filho e configurar os pipes
int fork_process(process_t* process, void (*child_function)(process_t*, VirtualMachine*), VirtualMachine* vm) {
    // Criar pipe para comunicação pai -> filho
    if (pipe(process->parent_to_child) == -1) {
        perror("Erro ao criar pipe pai -> filho");
        return -1;
    }

    // Criar pipe para comunicação filho -> pai
    if (pipe(process->child_to_parent) == -1) {
        perror("Erro ao criar pipe filho -> pai");
        close(process->parent_to_child[0]);  // Fechar as extremidades dos pipes
        close(process->parent_to_child[1]);
        return -1;
    }

    // Criar o processo filho
    process->pid = fork();
    if (process->pid == -1) {
        perror("Erro ao criar processo filho");
        close(process->parent_to_child[0]);  // Fechar as extremidades dos pipes
        close(process->parent_to_child[1]);
        close(process->child_to_parent[0]);
        close(process->child_to_parent[1]);
        return -1;
    }


    if (process->pid == 0) {
        // Processo filho: fechar extremidades dos pipes não utilizadas
        close(process->parent_to_child[1]);  // Fecha escrita do pai -> filho
        close(process->child_to_parent[0]);  // Fecha leitura do filho -> pai

        // Chamar a função do processo filho
        child_function(process, vm);

        // Fechar os pipes após a execução
        close(process->parent_to_child[0]);
        close(process->child_to_parent[1]);

        //exit(0);  // Termina o processo filho após a execução
    } else {
        // Processo pai: fechar extremidades dos pipes não utilizadas
        close(process->parent_to_child[0]);  // Fecha leitura do pai -> filho
        close(process->child_to_parent[1]);  // Fecha escrita do filho -> pai
    }

    return 0;
}

// create process but with fork + exec
int create_process(process_t* process, char* argv[]) 
{
    // Criar pipe para comunicação pai -> filho
    if (pipe(process->parent_to_child) == -1) {
        perror("Erro ao criar pipe pai -> filho");
        return -1;
    }

    // Criar pipe para comunicação filho -> pai
    if (pipe(process->child_to_parent) == -1) {
        perror("Erro ao criar pipe filho -> pai");
        close(process->parent_to_child[0]);  // Fechar as extremidades dos pipes
        close(process->parent_to_child[1]);
        return -1;
    }

    // Criar o processo filho
    process->pid = fork();
    if (process->pid == -1) {
        perror("Erro ao criar processo filho");
        close(process->parent_to_child[0]);  // Fechar as extremidades dos pipes
        close(process->parent_to_child[1]);
        close(process->child_to_parent[0]);
        close(process->child_to_parent[1]);
        return -1;
    }

    if (process->pid == 0) {
        // Processo filho: fechar extremidades dos pipes não utilizadas
        close(process->parent_to_child[1]);  // Fecha escrita do pai -> filho
        close(process->child_to_parent[0]);  // Fecha leitura do filho -> pai

        // Redirecionar a entrada e saída padrão
        dup2(process->parent_to_child[0], STDIN_FILENO);
        dup2(process->child_to_parent[1], STDOUT_FILENO);

        // Fechar os pipes após a execução
        close(process->parent_to_child[0]);
        close(process->child_to_parent[1]);

        // Executar o programa
        execvp(argv[0], argv);

        // Se execvp retornar, houve um erro
        perror("Erro ao executar o programa");
        exit(EXIT_FAILURE);
    } else {
        // Processo pai: fechar extremidades dos pipes não utilizadas
        close(process->parent_to_child[0]);  // Fecha leitura do pai -> filho
        close(process->child_to_parent[1]);  // Fecha escrita do filho -> pai
    }

    return 0;
}

/* create process exec child process example

*/

// Função para enviar uma string dinamicamente
void send_dynamic_string(process_t* process, const char* str, int to_parent) 
{
    size_t len = strlen(str) + 1;  // Inclui o null terminator

    if (to_parent) {
        // Enviar para o pai (pipe filho -> pai)
        if (write(process->child_to_parent[1], &len, sizeof(len)) == -1) {
            perror("Erro ao enviar tamanho da string para o pai");
            exit(EXIT_FAILURE);
        }
        if (write(process->child_to_parent[1], str, len) == -1) {
            perror("Erro ao enviar string para o pai");
            exit(EXIT_FAILURE);
        }
    } else {
        // Enviar para o filho (pipe pai -> filho)
        if (write(process->parent_to_child[1], &len, sizeof(len)) == -1) {
            perror("Erro ao enviar tamanho da string para o filho");
            exit(EXIT_FAILURE);
        }
        if (write(process->parent_to_child[1], str, len) == -1) {
            perror("Erro ao enviar string para o filho");
            exit(EXIT_FAILURE);
        }
    }
}

char* receive_dynamic_string(process_t* process, int from_parent) 
{
    size_t len;
    char* buffer;
    int fd = from_parent ? process->parent_to_child[0] : process->child_to_parent[0];

    // Estrutura para o poll()
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    // Verifica se há algo no pipe antes de bloquear
    int ret = poll(&pfd, 1, 1000);  // Timeout de 1 segundo

    if (ret == -1) 
    {
        perror("error verifying data in pipe");
        exit(EXIT_FAILURE);
    } 
    else if (ret == 0) 
    {
        printf("no data on pipe.\n");
        return NULL;  // Nenhum dado disponível
    }

    // Dados disponíveis, proceder com a leitura
    if (read(fd, &len, sizeof(len)) == -1) 
    {
        perror("error receiving string length");
        exit(EXIT_FAILURE);
    }

    buffer = (char*)malloc(len);
    if (buffer == NULL) 
    {
        perror("error allocating memory for string");
        exit(EXIT_FAILURE);
    }

    if (read(fd, buffer, len) == -1) 
    {
        perror("error receiving string");
        exit(EXIT_FAILURE);
    }

    return buffer;  // Retorna a string dinamicamente alocada
}

int is_pipe_open(int fd) 
{
    // Usa fcntl para verificar se o descritor de arquivo está aberto
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

void process_destroy(process_t* process) 
{
    // Verifica se os pipes estão abertos antes de tentar usá-los
    if (is_pipe_open(process->parent_to_child[1])) {
        send_dynamic_string(process, "destroy", 0);
        char* str = receive_dynamic_string(process, 0);
        free(str);
    } else {
        printf("Pipe pai->filho está fechado. Não é possível enviar 'destroy'.\n");
    }

    if (process->pid > 0) {
        // Espera o processo filho terminar
        if (waitpid(process->pid, NULL, WNOHANG) == 0) {
            // O processo ainda está rodando, então podemos esperar ele terminar
            waitpid(process->pid, NULL, 0);
        } else {
            printf("Processo já foi encerrado.\n");
        }
    }

    // Verifica se os pipes estão abertos e fecha-os se necessário
    if (is_pipe_open(process->parent_to_child[1])) {
        close(process->parent_to_child[1]);
    }
    if (is_pipe_open(process->parent_to_child[0])) {
        close(process->parent_to_child[0]);
    }
    if (is_pipe_open(process->child_to_parent[1])) {
        close(process->child_to_parent[1]);
    }
    if (is_pipe_open(process->child_to_parent[0])) {
        close(process->child_to_parent[0]);
    }
}

// Função do processo filho
void default_interpreter(process_t* process, VirtualMachine* vm) 
{
    int index = new_var(vm);
    vm->stack->data[index].process = process;
    vm->typestack->data[index] = TYPE_PROCESS;
    hold_var(vm, index);
    hash_set(vm, "process.child", index);
    char* _str;
    Int result = -1;
    
    while (1) 
    {
        char* received = receive_dynamic_string(process, 1);  
        if (!received) 
        {
            printf("error receiving string\n");
            continue;
        }
        if (strcmp(received, "destroy") == 0) 
        {
            free(received);
            break;  
        }
        
        result = eval(vm, received);  
        free(received);
        
        if (result > -1) 
        {
            if (vm->typestack->data[result] == TYPE_STRING) 
            {
                _str = vm->stack->data[result].string;
                send_dynamic_string(process, _str, 1);  
            } 
            else 
            {
                _str = str_format("%d", vm->stack->data[result].number);
                send_dynamic_string(process, _str, 1);  
                free(_str);
            }
        }
        else 
        {
            send_dynamic_string(process, "", 1);
        }
        
    }
    vm->stack->data[index].process = NULL;
    vm->typestack->data[index] = TYPE_NIL;
    send_dynamic_string(process, "destroyd", 1);
}

// process function declarations
Int std_process_fork(VirtualMachine *vm, IntList *args)
{
    Int name = -1;
    if (args->size > 0)
    {
        name = stack_shift(*args);
    }
    process_t *process = (process_t*)malloc(sizeof(process_t));
    // Criar o processo filho
    if (fork_process(process, default_interpreter, vm) == -1) 
    {
        perror("Erro ao criar processo");
        exit(EXIT_FAILURE);
    }

    Int result = new_var(vm);
    
    

    if (process->pid == 0) 
    {
        vm->stack->data[result].process = NULL;
        vm->typestack->data[result] = TYPE_NIL;
        vm->stack->data[result].integer = 0;
        free(process);
        return new_number(vm, 0);
    }
    vm->typestack->data[result] = TYPE_PROCESS;
    vm->stack->data[result].process = process;
    hold_var(vm, result);
    
    if (name >= 0)
    {
        hash_set(vm, vm->stack->data[name].string, result);
        return -1;
    }
    
    return result;
}

Int std_process_host_send(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    Int message = stack_shift(*args);
    send_dynamic_string((process_t*)vm->stack->data[process].process, vm->stack->data[message].string, 0);
    return -1;
}

Int std_process_host_receive(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    char *received = receive_dynamic_string((process_t*)vm->stack->data[process].process, 0);
    if (strcmp(received, "") == 0)
    {
        free(received);
        return -1;
    }
    else 
    {
        Int result = new_string(vm, received);
        free(received);
        return result;
    }
}

Int std_process_host_await(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    char *received = receive_dynamic_string((process_t*)vm->stack->data[process].process, 0);
    while (strcmp(received, "") == 0)
    {
        free(received);
        received = receive_dynamic_string((process_t*)vm->stack->data[process].process, 0);
    }
    Int result = new_string(vm, received);
    free(received);
    return result;
}

Int std_process_child_send(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    Int message = stack_shift(*args);
    send_dynamic_string((process_t*)vm->stack->data[process].process, vm->stack->data[message].string, 1);
    return -1;
}

Int std_process_child_receive(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    char *received = receive_dynamic_string((process_t*)vm->stack->data[process].process, 1);
    Int result = eval(vm, received);
    free(received);
    return result;
}

Int std_process_destroy(VirtualMachine *vm, IntList *args)
{
    Int process = stack_shift(*args);
    process_destroy((process_t*)vm->stack->data[process].process);
    unuse_var(vm, process);
    return -1;
}

// inits

void init_linux_process(VirtualMachine *vm)
{
    hold_var(vm,spawn_builtin(vm, "process.fork", std_process_fork));
    //hold_var(vm,spawn_builtin(vm, "process.create", std_process_create));
    hold_var(vm,spawn_builtin(vm, "process.send", std_process_host_send));
    hold_var(vm,spawn_builtin(vm, "process.await", std_process_host_await));
    hold_var(vm,spawn_builtin(vm, "process.destroy", std_process_destroy));
    hold_var(vm,spawn_builtin(vm, "process.receive", std_process_host_receive));
    hold_var(vm,spawn_builtin(vm, "process.child.send", std_process_child_send));
    hold_var(vm,spawn_builtin(vm, "process.child.receive", std_process_child_receive));
}


void init_linux(VirtualMachine *vm)
{
    init_linux_process(vm);
}