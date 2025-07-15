fn foo2() {
    return;
}

fn foo(mut a : i32, mut b : i32) -> i32 {
    let mut c = a + b;
    return a + b + c;
}

fn main0() -> i32 {
    let mut a = 1;
    let mut b = 2;

    return foo(a, b);
}
