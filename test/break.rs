fn main() -> i32 {
    let mut a : i32 = 1;
    let mut b : (i32, [i32 ; 2], ) = (1, 1, );
    if a > 1 {
        break;
    }

    while a {
        return 1;
    }

    for i in [1, 2, 3] {
        return 1;
    }

    loop {
    if a > 1 {
        return 1;
    } else {
        return 2;
    }
    }
}
