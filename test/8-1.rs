fn main(){
    // let mut a:[i32;3];
    // let mut b:[&i32;3];
    // let mut c:[[&i32;3];3];
    // let mut d:&mut[[i32;3];3];
    let mut a:[i32;3];
    let mut b:[i32;3];
    let mut c:[[i32;3];3];
    let mut d:[[i32;3];3];
    a = [1,2,3];
}

//注意，理论上这些嵌套是不成立的，因为必须对类型所处有明确的赋值，但是暂时先保证能识别嵌套。
