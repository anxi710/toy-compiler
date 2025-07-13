fn foo(mut a : [i32 ; 3], mut b : i32) -> [i32 ; 3] {
    for mut i in a {
        let mut c = i;
    }

    for mut i in 1 .. 4 {
        loop {
            break 1;
        };
        loop {
            break 1;
        }
        loop {
            break;
        }
    }

    return [1, 2,];
    return [[1, 2, 3], [4, 5, 6], [7, 8, 9]][1];
    return [(1, 2, 3), (4, 5, 6), (7, 8, 9)][1];
    return ([1, 2, 3][1], [2, 2, 1], 1).0;
    return ([1, 2, 3][1], [2, 2, 1], 1).1;
    return ([(1, 2, 3), (4, 5, 6), (7, 8, 9)], 1, 2).0[1];
}

fn main() -> i32 {
    let mut a : i32 = 0;
    for i in foo() {
        a = a + i;
    }

    return a;
}
