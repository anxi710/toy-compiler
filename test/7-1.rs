fn main(mut x: i32, mut y : i32) {
    let mut z = {
        let mut t = x * x + x;
        t = t + x * y;
        t + x * y;
        t
    };
}
