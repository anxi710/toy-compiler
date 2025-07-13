fn main() -> i32 {
    let mut a = 1;
    let mut b = if a > 1 {
        1
    } else if a > 2 {
        return 2;
    } else {
        return 1;
    };
    let mut c : () = b;

    return loop { break 1 }
    return if a > 1 {
        1
    };
    return if a > 1 {
        1
    } else if {
        return 1;
    };
    return if a > 1 {
        1
    } else if {
        return 1;
    } else {
        2
    };
}