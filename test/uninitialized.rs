fn foo(mut a : i32, mut b : i32) {

}

fn main() -> i32 {
    let mut a : i32 = 1;
    let mut b : i32;
    b;
    (b);
    (b + a);
    (b == a);
    foo(a, b);
    foo(b, a);
    loop { break b; }
    if b == a {} else if b == a {}
    while b == a {}
    for i in a .. b {}

    b = a;

    b;
    (b);
    (b + a);
    (b == a);
    foo(a, b);
    foo(b, a);
    loop { break b; }
    if b == a {} else if b == a {}
    while b == a {}
    for i in a .. b {}

    let mut b : [i32 ; 2];
    b[1] = a;

    let mut b : [i32 ; 2];
    b.0 = a;

    let mut d : [i32 ; 2];
    for i in d {}
    d = [1, 2];
    for i in d {}

    let mut e;
    return e;
}
