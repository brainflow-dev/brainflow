mod common;

#[test]
fn get_data() {
    let data = common::board_data(Some(10));

    println!("{}", data.len());
    println!("{:?}", data);
    assert_eq!(data.len(), 32);

    // check that the first array in data are sequential numbers
    for (d, d_plus_1) in data[0].iter().zip(data[0].iter().skip(1)) {
        assert_eq!(d + 1.0, *d_plus_1);
    }
}
