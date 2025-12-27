//&T-
ret;

func1(): integer
begin
    // [CORRECT]
    return 123;

    // [ERROR] incompatible type
    return 456.0;

    // [ERROR] incompatible type
    return "789";
end
end

func2(): real
begin
    // [CORRECT]
    return 123;

    // [CORRECT]
    return 456.0;

    // [ERROR] incompatible type
    return "789";
end
end

func3(): boolean
begin
    var bool: array 10 of array 100 of boolean;

    // [ERROR] incompatible type
    return bool[1];

    // [CORRECT]
    return bool[1][1];

    // [ERROR] over subscript
    return bool[1][1][1];
end
end

begin
    // [ERROR] program doesn't return a value
    return 10;
end
end
