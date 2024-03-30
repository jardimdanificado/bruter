local _string = {}

_string.endsWith = function(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

_string.charAt = function(str, index)
    return str.sub(str,index, index)
end

_string.byteAt = function(str, index)
    return string.byte(str, index)
end

_string.split = function(str, separator)
    local parts = {}
    local start = 1
    separator = separator or ''
    if separator == '' then
        for i = 1, #str do
            parts[i] = string.sub(str, i, i)
        end
        return parts
    end
    local splitStart, splitEnd = string.find(str, separator, start)
    while splitStart do
        table.insert(parts, string.sub(str, start, splitStart - 1))
        start = splitEnd + 1
        splitStart, splitEnd = string.find(str, separator, start)
    end
    table.insert(parts, string.sub(str, start))
    return parts
end

_string.split2 = function(str, separator) -- returns a table of strings respecting the backticks and removing them
    local result = {}
    local current = ""
    local insideString = false

    for i = 1, #str do
        local char = str:sub(i, i)

        if char == "`" then
            insideString = not insideString
        elseif char == separator and not insideString then
            table.insert(result, current)
            current = ""
        else
            current = current .. char
        end
    end

    table.insert(result, current)
    return result
end

_string.split3 = function(str, separator) -- returns a table with the parts of the string respecting the backticks and keeping them
    local result = {}
    local current = ""
    local insideString = false

    for i = 1, #str do
        local char = str:sub(i, i)

        if char == "`" then
            insideString = not insideString
            current = current .. char
        elseif char == separator and not insideString then
            table.insert(result, current)
            current = ""
        else
            current = current .. char
        end
    end

    table.insert(result, current)

    return result
end


_string.replace = function(inputString, oldSubstring, newSubstring)
    newSubstring = newSubstring or ''
    return inputString:gsub(oldSubstring, newSubstring)
end

_string.replace2 = function(inputString, oldSubstring, newSubstring) -- returns a string with the oldSubstring replaced by the newSubstring respecting the backticks enclosed strings
    local result = ""
    local insideString = false
    local current = ""

    for i = 1, #inputString do
        local char = inputString:sub(i, i)

        if char == "`" then
            insideString = not insideString
            current = current .. char
        elseif not insideString and char == oldSubstring:sub(1, 1) then
            if current ~= "" then
                result = result .. current
                current = ""
            end

            if inputString:sub(i, i + #oldSubstring - 1) == oldSubstring then
                result = result .. newSubstring
                i = i + #oldSubstring - 1
            else
                result = result .. char
            end
        else
            current = current .. char
        end
    end

    result = result .. current

    return result
end

_string.replace3 = function(inputString, oldSubstring, newSubstring) -- returns a string with the oldSubstring replaced by the newSubstring respecting the backticks enclosed strings and keeping them
    local result = ""
    local insideString = false
    local current = ""

    for i = 1, #inputString do
        local char = inputString:sub(i, i)

        if char == "`" then
            insideString = not insideString
            current = current .. char
        elseif not insideString and char == oldSubstring:sub(1, 1) then
            if current ~= "" then
                result = result .. current
                current = ""
            end

            if inputString:sub(i, i + #oldSubstring - 1) == oldSubstring then
                result = result .. newSubstring
                i = i + #oldSubstring - 1
            else
                result = result .. char
            end
        else
            current = current .. char
        end
    end

    result = result .. current

    return result
end


_string.includes = function(str, substring)
    return string.find(str, substring, 1, true) ~= nil
end

_string.trim = function(s)
    return s:gsub("^%s+", ""):gsub("%s+$", "")
end

return _string