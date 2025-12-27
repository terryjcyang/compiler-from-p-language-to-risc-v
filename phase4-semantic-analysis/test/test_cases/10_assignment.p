//&T-
assignment;

begin
    var const: 10;
    var arr: array 20 of array 777 of real;
    var float: real;
    var bool: boolean;

    // [ERROR] symbol not found
    err := arr[12][2];

    // [ERROR] array assignment (variable reference)
    arr := 1.23;

    // [ERROR] const assignment
    const := "SSLAB";

    for i := 1 to 30 do
    begin
        // [ERROR] loop variable assignment
        i := 213;
    end
    end do

    // [ERROR] invalid operand
    // [ERROR] const assignment
    const := "NO.1" + float;

    // [ERROR] const assignment
    const := arr;

    // [ERROR] array assignment (expression)
    float := arr[10];

    // [ERROR] type mismatch
    arr[0][23] := false;
    float := "string";

    // [CORRECT]
    float := 4210;

    // [ERROR] invalid operand
    bool := -false;
end
end
