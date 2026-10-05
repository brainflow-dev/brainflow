import {DataFilter} from 'brainflow';

// Two simultaneous mixtures: rows are channels and columns are samples.
const samples = 1024;
const data: number[][] = [[], []];
for (let i = 0; i < samples; i++)
{
    const t = i / 256;
    const first = Math.sin(2 * Math.PI * 7 * t);
    const second = Math.pow(Math.sin(2 * Math.PI * 13 * t), 3);
    data[0].push(first + 0.3 * second);
    data[1].push(0.2 * first + second);
}
const ica = DataFilter.performIca(data, 2, [0, 1]);
// Component order and sign are arbitrary.
console.info(`Recovered ${ica[3].length} sources from ${samples} samples`);
