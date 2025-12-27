//&T-
FunctionInvocation;

func() : boolean
begin
    return true;
end
end

func1(a: integer)
begin
end
end

func2(a: real)
begin
end
end

func3(a: array 7 of integer)
begin
end
end

func4(a: array 7 of real)
begin
end
end

func5(a: integer; b: string; c: array 7 of integer)
begin
end
end

begin
    var integerArr: array 7 of integer;
    var realArr: array 7 of real;
    var booleanArr: array 7 of boolean;

    // [ERROR] identifier has to be in symbol tables
    null(123, 456);

    // [ERROR] kind of symbol has to be function
    arr(123, 456);

    // [ERROR] # of arguments isn't the same as parameters
    func("overflow");

    // [CORRECT]
    func1(123);

    // [ERROR] incompatible argument and parameter types
    func1(456.0);

    // [ERROR] incompatible argument and parameter types
    func1("789");

    // [CORRECT]
    func2(123);

    // [CORRECT]
    func2(456.0);

    // [ERROR] incompatible argument and parameter types
    func2("789");

    // [CORRECT]
    func3(integerArr);

    // [ERROR] incompatible argument and parameter types
    func3(realArr);

    // [ERROR] incompatible argument and parameter types
    func3(booleanArr);

    // [ERROR] incompatible argument and parameter types
    func4(integerArr);

    // [CORRECT]
    func4(realArr);

    // [ERROR] incompatible argument and parameter types
    func4(booleanArr);

    // [ERROR] incompatible argument and parameter types
    func5(123, 456, integerArr);

    // [CORRECT]
    func5(123, "456", integerArr);

    // [ERROR] incompatible argument and parameter types
    func5(123, "456", realArr);

    // [ERROR] invalid operand
    func5(-false, "456", integerArr);
end
end
