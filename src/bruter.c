#include "bruter.h"

// parser and interpreter declarations
IntList *default_parser(void *_vm, char *cmd);
Int default_interpreter(void *_vm, char *cmd);

//string functions
char* str_duplicate(const char *str)
{
    char *dup = (char*)malloc(strlen(str) + 1);
    strcpy(dup, str);
    return dup;
}

char* str_nduplicate(const char *str, Int n)
{
    char *dup = (char*)malloc(n + 1);
    for (Int i = 0; i < n; i++)
    {
        dup[i] = str[i];
    }
    dup[n] = '\0';
    return dup;
}

char* str_sub(const char *str, Int start, Int end)
{
    char *sub = (char*)malloc(end - start + 1);
    for (Int i = start; i < end; i++)
    {
        sub[i - start] = str[i];
    }
    sub[end - start] = '\0';
    return sub;
}

char* str_format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Int size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *str = (char*)malloc(size + 1);
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    return str;
}

char* str_concat(const char *str1, const char *str2)
{
    char *str = (char*)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(str, str1);
    strcat(str, str2);
    return str;
}

char* str_replace(const char *str, const char *substr, const char *replacement)
{
    Int len = strlen(str);
    Int sublen = strlen(substr);
    Int replen = strlen(replacement);
    
    // Encontrar a primeira ocorrência de 'substr'
    Int i = 0;
    Int pos = -1;  // Posição da primeira ocorrência
    while (str[i] != '\0')
    {
        if (str[i] == substr[0])
        {
            Int j = 0;
            while (j < sublen && str[i + j] == substr[j])
            {
                j++;
            }
            if (j == sublen)
            {
                pos = i;
                break;  // Encontrou a primeira ocorrência, sair do loop
            }
        }
        i++;
    }

    // Se não encontrou a substring, retornar a string original
    if (pos == -1)
    {
        char *newstr = (char*)malloc(len + 1);
        strcpy(newstr, str);
        return newstr;
    }

    // Calcular o novo tamanho e alocar memória para a nova string
    char *newstr = (char*)malloc(len - sublen + replen + 1);

    // Copiar a parte antes da substring encontrada
    strncpy(newstr, str, pos);
    
    // Copiar o replacement
    strcpy(newstr + pos, replacement);
    
    // Copiar o restante da string após a substring substituída
    strcpy(newstr + pos + replen, str + pos + sublen);
    
    return newstr;
}


char* str_replace_all(const char *str, const char *substr, const char *replacement)
{
    Int len = strlen(str);
    Int sublen = strlen(substr);
    Int replen = strlen(replacement);
    Int count = 0;
    for (Int i = 0; i < len; i++)
    {
        if (str[i] == substr[0])
        {
            Int j = 0;
            while (j < sublen && str[i + j] == substr[j])
            {
                j++;
            }
            if (j == sublen)
            {
                count++;
            }
        }
    }
    char *newstr = (char*)malloc(len + count * (replen - sublen) + 1);
    Int i = 0;
    Int k = 0;
    while (str[i] != '\0')
    {
        if (str[i] == substr[0])
        {
            Int j = 0;
            while (j < sublen && str[i + j] == substr[j])
            {
                j++;
            }
            if (j == sublen)
            {
                for (Int l = 0; l < replen; l++)
                {
                    newstr[k] = replacement[l];
                    k++;
                }
                i += sublen;
            }
            else
            {
                newstr[k] = str[i];
                k++;
                i++;
            }
        }
        else
        {
            newstr[k] = str[i];
            k++;
            i++;
        }
    }
    newstr[k] = '\0';
    return newstr;
}

Int str_find(const char *str, const char *substr)
{
    return strstr(str, substr) - str;
}

StringList* special_space_split(char *str)
{
    StringList *splited;
    splited = (StringList*)malloc(sizeof(StringList));
    stack_init(*splited);
    
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '(')
        {
            int j = i;
            int count = 1;
            while (count != 0)
            {
                j++;
                if (str[j] == '(')
                {
                    count++;
                }
                else if (str[j] == ')')
                {
                    count--;
                }
            }
            char *tmp = str_nduplicate(str + i, j - i + 1);
            stack_push(*splited, tmp);
            i = j + 1;
        }
        else if (str[i] == '"')
        {
            int j = i;
            j++;  // Avança para depois da abertura de aspas duplas
            while (str[j] != '"' && str[j] != '\0')
            {
                j++;
            }
            char *tmp = str_nduplicate(str + i, j - i + 1);
            stack_push(*splited, tmp);
            i = j + 1;  // Avança para após o fechamento de aspas duplas
        }
        else if (str[i] == '\'')
        {
            int j = i;
            j++;  // Avança para depois da abertura de aspas simples
            while (str[j] != '\'' && str[j] != '\0')
            {
                j++;
            }
            char *tmp = str_nduplicate(str + i, j - i + 1);
            stack_push(*splited, tmp);
            i = j + 1;  // Avança para após o fechamento de aspas simples
        }
        else if (is_space(str[i]))
        {
            i++;
        }
        else
        {
            int j = i;
            while (!is_space(str[j]) && str[j] != '\0' && str[j] != '(' && str[j] != ')' && str[j] != '"' && str[j] != '\'')
            {
                j++;
            }
            stack_push(*splited, str_nduplicate(str + i, j - i));
            i = j;
        }
    }
    return splited;
}


StringList* special_split(char *str, char delim)
{
    StringList *splited;
    splited = (StringList*)malloc(sizeof(StringList));
    stack_init(*splited);
    
    int recursion = 0;
    int inside_double_quotes = 0;
    int inside_single_quotes = 0;
    int i = 0;
    int last_i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == '(' && inside_double_quotes == 0 && inside_single_quotes == 0)
        {
            recursion++;
        }
        else if (str[i] == ')' && inside_double_quotes == 0 && inside_single_quotes == 0)
        {
            recursion--;
        }
        else if (str[i] == '"' && recursion == 0 && inside_single_quotes == 0)
        {
            // Alterna o estado de dentro/fora de aspas duplas
            inside_double_quotes = !inside_double_quotes;
        }
        else if (str[i] == '\'' && recursion == 0 && inside_double_quotes == 0)
        {
            // Alterna o estado de dentro/fora de aspas simples
            inside_single_quotes = !inside_single_quotes;
        }

        // Se encontramos o delimitador e não estamos dentro de parênteses nem de aspas simples ou duplas
        if (str[i] == delim && recursion == 0 && inside_double_quotes == 0 && inside_single_quotes == 0)
        {
            char* tmp = str_nduplicate(str + last_i, i - last_i);
            stack_push(*splited, tmp);
            last_i = i + 1;
        }
        else if (str[i + 1] == '\0') // Checagem para o último token
        {
            char* tmp = str_nduplicate(str + last_i, i - last_i + 1);
            stack_push(*splited, tmp);
        }

        i++;
    }
    return splited;
}

StringList* str_split(char *str, char *delim)
{
    StringList *splited;
    splited = (StringList*)malloc(sizeof(StringList));
    stack_init(*splited);
    
    Int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == delim[0])
        {
            i++;
        }
        else
        {
            Int j = i;
            while (str[j] != delim[0] && str[j] != '\0')
            {
                j++;
            }
            stack_push(*splited, str_nduplicate(str + i, j - i));
            i = j;
        }
    }

    return splited;
}


StringList* str_split_char(char *str, char delim)
{
    StringList *splited;
    splited = (StringList*)malloc(sizeof(StringList));
    stack_init(*splited);

    Int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == delim)
        {
            i++;
        }
        else
        {
            Int j = i;
            while (str[j] != delim && str[j] != '\0')
            {
                j++;
            }
            stack_push(*splited, str_nduplicate(str + i, j - i));
            i = j;
        }
    }

    return splited;
}

#ifndef ARDUINO

// print 
void print_element(VirtualMachine *vm, Int index)
{
    if (index < 0 || index >= vm->stack->size)
    {
        printf("{invalid}");
        return;
    }

    Value temp = vm->stack->data[index];

    switch (data_t(index))
    {
        case TYPE_NIL:
            printf("{nil}");
            break;
        case TYPE_INTEGER:
            printf("%ld", data(index).integer);
            break;
        case TYPE_BUILTIN:
            printf("%ld", data(index).pointer);
            break;
        case TYPE_LIST:
            printf("[");
            IntList *list = (IntList*)data(index).pointer;
            for (Int i = 0; i < list->size; i++)
            {
                printf("%ld", list->data[i]);
                if (i < list->size - 1)
                {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case TYPE_OTHER:
            printf("%ld", data(index).pointer);
            break;
        case TYPE_NUMBER:
            if (round(data(index).number) == data(index).number)
            {
                printf("%ld", (Int)data(index).number);
            }
            else
            {
                printf("%f", data(index).number);
            }
            break;
        case TYPE_STRING:
            printf("%s", data(index).string);
            break;

        default:
            printf("{unknown} %d", data(index).pointer);
            break;
    }
}

// repl 
Int repl(VirtualMachine *vm)
{
    printf("bruter v%s\n", VERSION);
    char *cmd;
    Int result = -1;
    int junk = 0;
    while (result == -1)
    {
        cmd = (char*)malloc(1024);
        printf("@> ");
        junk = scanf("%[^\n]%*c", cmd);
        if (junk == 0)
        {
            free(cmd);
            break;
        }
        result = eval(vm, cmd);
        free(cmd);
    }

    printf("repl returned: @%ld\n", result);
    print_element(vm, result);
    printf("\n");
}

// file functions
char* readfile(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return NULL;
    }
    char *code = (char*)malloc(1);
    code[0] = '\0';
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1)
    {
        code = (char*)realloc(code, strlen(code) + strlen(line) + 1);
        strcat(code, line);
    }
    free(line);
    fclose(file);
    return code;
};

void writefile(char *filename, char *code)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        return;
    }
    fprintf(file, "%s", code);
    fclose(file);
}

#endif

// value functions

Value value_duplicate(Value value, char type)
{
    Value dup;

    switch (type)
    {
        case TYPE_STRING:
            dup.string = str_duplicate(value.string);
            break;
        case TYPE_LIST:
            dup.pointer = make_int_list();
            IntList *list = (IntList*)value.pointer;
            for (Int i = 0; i < list->size; i++)
            {
                stack_push(*((IntList*)dup.pointer), list->data[i]);
            }
            break;
        case TYPE_BUILTIN:
            dup.pointer = value.pointer;
            break;
        default:
            dup.integer = value.integer;
            break;
    }

    return dup;
}

//hash functions

Int hash_find(VirtualMachine *vm, char *varname)
{
    for (Int i = 0; i < vm->hashes->size; i++)
    {
        if (strcmp(vm->hashes->data[i].key, varname) == 0)
        {
            return vm->hashes->data[i].index;
        }
    }
    return -1;
}

void hash_set(VirtualMachine *vm, char* varname, Int index)
{
    Int _index = hash_find(vm, varname);
    if (_index != -1)
    {
        vm->hashes->data[_index].index = index;
    }
    else 
    {
        Hash hash;
        hash.key = str_duplicate(varname);
        hash.index = index;
        stack_push(*vm->hashes, hash);
    }
}

void hash_unset(VirtualMachine *vm, char* varname)
{
    Int index = hash_find(vm, varname);
    if (index != -1)
    {
        free(vm->hashes->data[index].key);
        vm->hashes->data[index].key = NULL;
        stack_remove(*vm->hashes, index);
    }
}

//variable functions

ValueList* make_value_list()
{
    ValueList *list = (ValueList*)malloc(sizeof(ValueList));
    stack_init(*list);
    return list;
}

IntList* make_int_list()
{
    IntList *list = (IntList*)malloc(sizeof(IntList));
    stack_init(*list);
    return list;
}

StringList* make_string_list()
{
    StringList *list = (StringList*)malloc(sizeof(StringList));
    stack_init(*list);
    return list;
}

CharList* make_char_list()
{
    CharList *list = (CharList*)malloc(sizeof(CharList));
    stack_init(*list);
    return list;
}

VirtualMachine* make_vm()
{
    VirtualMachine *vm = (VirtualMachine*)malloc(sizeof(VirtualMachine));
    vm->stack = make_value_list();
    vm->typestack = make_char_list();
    vm->hashes = (HashList*)malloc(sizeof(HashList));
    stack_init(*vm->hashes);
    vm->interpret = default_interpreter;
    vm->unused = make_int_list();
    return vm;
}

// var new 
Int new_var(VirtualMachine *vm)
{ 
    if (vm->unused->size > 0)
    {
        Int index = stack_pop(*vm->unused);
        unuse_var(vm, index);
        return index;
    }
    else
    {
        Value value;
        value.pointer = NULL;
        stack_push(*vm->stack, value);
        stack_push(*vm->typestack, TYPE_OTHER);
        return vm->stack->size-1;
    }
}

Int new_number(VirtualMachine *vm, Float number)
{
    Int id = new_var(vm);
    vm->stack->data[id].number = number;
    vm->typestack->data[id] = TYPE_NUMBER;
    return id;
}

Int new_string(VirtualMachine *vm, char *string)
{
    Int id = new_var(vm);
    vm->stack->data[id].string = str_duplicate(string);
    vm->typestack->data[id] = TYPE_STRING;
    return id;
}

Int new_builtin(VirtualMachine *vm, Function function)
{
    Int id = new_var(vm);
    vm->stack->data[id].pointer = function;
    vm->typestack->data[id] = TYPE_BUILTIN;
    return id;
}

Int new_list(VirtualMachine *vm)
{
    Int id = new_var(vm);
    vm->stack->data[id].pointer = make_int_list();
    vm->typestack->data[id] = TYPE_LIST;
    return id;
}

// var spawn

Int spawn_var(VirtualMachine *vm, char* varname)
{
    Int index = new_var(vm);
    hash_set(vm, varname, index);
    return index;
}

Int spawn_number(VirtualMachine *vm, char* varname, Float number)
{
    Int index = new_number(vm, number);
    hash_set(vm, varname, index);
    return index;
}

Int spawn_string(VirtualMachine *vm, char* varname, char* string)
{
    Int index = new_string(vm, string);
    hash_set(vm, varname, index);
    return index;
}

Int spawn_builtin(VirtualMachine *vm, char* varname, Function function)
{
    Int index = new_builtin(vm, function);
    hash_set(vm, varname, index);
    return index;
}

Int spawn_list(VirtualMachine *vm, char* varname)
{
    Int index = new_list(vm);
    hash_set(vm, varname, index);
    return index;
}

// var register

void register_builtin(VirtualMachine *vm, char *name, Function function)
{
    spawn_builtin(vm, name, function);
}


void register_number(VirtualMachine *vm, char *name, Float number)
{
    spawn_number(vm, name, number);
}

void register_string(VirtualMachine *vm, char *name, char *string)
{
    spawn_string(vm, name, string);
}

void register_list(VirtualMachine *vm, char *name)
{
    spawn_list(vm, name);
}

//frees

void free_var(VirtualMachine *vm, Int index)
{
    switch (vm->typestack->data[index])
    {
        case TYPE_STRING:
            free(vm->stack->data[index].string);
            break;
        case TYPE_LIST:
            while (((IntList*)vm->stack->data[index].pointer)->size > 0)
            {
                stack_shift(*((IntList*)vm->stack->data[index].pointer));
            }
            stack_free(*((IntList*)vm->stack->data[index].pointer));
            break;
    }
    
    stack_remove(*vm->stack, index);
    stack_remove(*vm->typestack, index);
}

void free_vm(VirtualMachine *vm)
{
    while (vm->stack->size > 0)
    {
        free_var(vm, 0);
    }

    while (vm->hashes->size > 0)
    {
        Hash hash = stack_shift(*vm->hashes);
        free(hash.key);
    }

    stack_free(*vm->unused);

    stack_free(*vm->stack);
    stack_free(*vm->typestack);

    stack_free(*vm->hashes);

    free(vm);
}

// Parser functions
IntList* parse(void *_vm, char *cmd) 
{
    VirtualMachine* vm = (VirtualMachine*)_vm;
    IntList *result = make_int_list();
    StringList *splited = special_space_split(cmd);
    //Int current = 0;
    while (splited->size > 0)
    {
        char* str = stack_shift(*splited);
        if (str[0] == '(')
        {
            if(str[1] == '@' && str[2] == '@')//string
            {
                char* temp = str + 3;
                temp[strlen(temp) - 1] = '\0';
                Int var = new_string(vm, temp);
                stack_push(*result, var);            
            }
            else if(str[1] == '/' && str[2] == '/') // a comment
            {}
            else
            {
                char* temp = str + 1;
                temp[strlen(temp) - 1] = '\0';
                Int index = eval(vm, temp);
                stack_push(*result, index);
                //free(temp);
            }
        }
        else if (str[0] == '@') 
        {
            if (strchr(str, ':') != NULL)
            {
                StringList *splited = str_split(str, ":");
                char* s_left =  splited->data[0] + 1;
                char* s_right = splited->data[1];

                Int n_left = atoi(s_left);
                Int n_right = atoi(s_right);
                
                if (n_left == 0 && strlen(s_left) > 0)
                {
                    n_left = hash_find(vm, s_left);
                    
                }

                if (n_right == 0 && strlen(s_right) > 0)
                {
                    n_right = hash_find(vm, s_right);
                    
                }
                
                if (n_right == -1)
                {
                    n_right = vm->stack->size;
                }

                if (n_left == -1)
                {
                    n_left = 0;
                }
                
                if (n_left < n_right)
                {
                    for (Int i = n_left; i <= n_right; i++)
                    {
                        stack_push(*result, i);
                    }
                }
                else if (n_left > n_right)
                {
                    for (Int i = n_left; i >= n_right; i--)
                    {
                        stack_push(*result, i);
                    }
                }
                else 
                {
                    stack_push(*result, n_left);
                }

                while (splited->size > 0)
                {
                    free(stack_shift(*splited));
                }

                stack_free(*splited);
            }
            else
            {
                stack_push(*result, atoi(str + 1));
            }
        }
        else if (str[0] == '"' || str[0] == '\'') // string
        {
            char* temp = str + 1;
            temp[strlen(temp) - 1] = '\0';
            Int var = new_string(vm, temp);
            stack_push(*result, var);
        }
        else if ((str[0] >= '0' && str[0] <= '9') || str[0] == '-') 
        {
            Int var = new_number(vm, atof(str));
            stack_push(*result, var);
        }
        else //variable 
        {
            Int index = hash_find(vm, str);
            if (index == -1) 
            {
                printf("Variable %s not found\n", str);
                stack_push(*result, -1);
            }
            else 
            {
                stack_push(*result, index);
            }
        }
        free(str);
    }
    stack_free(*splited);
    return result;
}

Int default_interpreter(void *_vm, char* cmd)
{
    VirtualMachine* vm = (VirtualMachine*) _vm;
    IntList *args = parse(vm, cmd);
    Int func = stack_shift(*args);
    Int result = -1;

    if (func > -1 && vm->typestack->data[func] == TYPE_BUILTIN)
    {
        Function _function = vm->stack->data[func].pointer;
        result = _function(vm, args);
    }
    else 
    {
        printf("Error: %d is not a function\n", func);
    }

    stack_free(*args);
    return result;
}

Int eval(VirtualMachine *vm, char *cmd)
{
    if(strchr(cmd, ';') == NULL)
    {
        return vm->interpret(vm, cmd);
    }

    StringList *splited = special_split(cmd, ';');

    // remove empty or whitespace only strings using is_space
    Int last = splited->size - 1;
    while (last >= 0)
    {
        if (strlen(splited->data[last]) == 0)
        {
            free(splited->data[last]);
            stack_remove(*splited, last);
        }
        else
        {
            Int i = 0;
            while (splited->data[last][i] != '\0' && is_space(splited->data[last][i]))
            {
                i++;
            }
            if (splited->data[last][i] == '\0')
            {
                free(splited->data[last]);
                stack_remove(*splited, last);
            }
        }
        last--;
    }


    Int result = -1;
    while (splited->size > 0)
    {
        
        char *str = stack_shift(*splited);
        if (strlen(str) == 0)
        {
            free(str);
            continue;
        }
        result = vm->interpret(vm, str);
        free(str);
        if (result > 0)
        {
            while (splited->size > 0)
            {
                free(stack_shift(*splited));
            }
            break;
        }
    }
    stack_free(*splited);
    return result;
}

void unuse_var(VirtualMachine *vm, Int index)
{
    //if type is string free the string, if type is list free the list
    if (vm->typestack->data[index] == TYPE_STRING)
    {
        free(vm->stack->data[index].string);
    }
    else if (vm->typestack->data[index] == TYPE_LIST)
    {
        stack_free(*((IntList*)vm->stack->data[index].pointer));
    }

    data_t(index) = TYPE_NIL;
    
    for (Int i = 0; i < vm->hashes->size; i++)
    {
        if (vm->hashes->data[i].index == index)
        {
            free(vm->hashes->data[i].key);
            stack_remove(*vm->hashes, i);
        }
    }
    stack_push(*vm->unused, index);
}