//&T-

condition;
begin
    var int : integer;

    // [ERROR] invalid operand
    if int + "SSLAB" then
    begin
    end
    end if

    // [ERROR] condition must be boolean type
    if int + 10 then
    begin
    end
    end if

    // [CORRECT]
    while int > 10 do
    begin
    end
    end do

    // [ERROR] invalid operand
    while true > false do
    begin
    end
    end do
end
end
