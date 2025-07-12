fn foo(mut a : [i32 ; 3], mut b : i32) -> [i32 ; 3] {
    for mut i in a {
        let mut c = i;
    }

    return a;
}

fn main() -> i32 {
    let mut a : i32 = 0;
    for i in foo() {
        a = a + i;
    }

    return a;
}
