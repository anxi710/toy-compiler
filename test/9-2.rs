fn main(mut a:(i32,i32)){
    let mut b:i32=a.0;
    a.0=1;
    a.1=6*9;
    a.0=a.1;
}