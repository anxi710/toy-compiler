fn main() -> i32 {
    let mut a = 1;
    let mut b = if a > 1 {
        return 1;
    } else if a > 2 {
        return 2;
    } else {
        return 1;
    };
    let mut c : () = b;
}