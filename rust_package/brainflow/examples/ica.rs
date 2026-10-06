use brainflow::data_filter;
use ndarray::Array2;
use std::f64::consts::PI;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Two simultaneous mixtures: rows are channels and columns are samples.
    let data = Array2::from_shape_fn((2, 1024), |(row, col)| {
        let t = col as f64 / 256.0;
        let first = (2.0 * PI * 7.0 * t).sin();
        let second = (2.0 * PI * 13.0 * t).sin().powi(3);
        if row == 0 { first + 0.3 * second } else { 0.2 * first + second }
    });
    let (_, _, _, sources) = data_filter::perform_ica(data, 2)?;
    // Component order and sign are arbitrary.
    println!("Recovered 2 sources from {} samples", sources.len() / 2);
    Ok(())
}
