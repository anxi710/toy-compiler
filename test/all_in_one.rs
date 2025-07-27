fn foo4() -> i32 {
    return loop { break 2 }
}

fn foo3() -> i32 {
    let mut a = 1;
    let mut b = if a > 1 {
        1
    } else {
        2
    };

    while a >= 1 {
        a = a - 1;
        b = b / 1;
    }

    a = 4;
    while a >= 1 {
        if a >= 1 {
            a = a - 1;
            continue;
        }
        b = 10;
    }

    return b;
}

fn foo2() -> i32 {
    let mut a = 1;
    a = a * 4;

    if a > 1 {
        a = a + 1;
    }

    let mut b = 0;
    for mut i in 1..a {
        b = b + i;
    }

    b
}

fn foo(mut a : i32, mut b : i32) -> i32 {
    let mut c = a + b;
    return a + b + c;
}

fn main0() -> i32 {
    let mut a = 1;
    let mut b = 2;
    // 6 + 1 + 2 + 3 + 4 + 2 + 2
    return foo(a, b) + foo2() + foo3() + foo4();
}
