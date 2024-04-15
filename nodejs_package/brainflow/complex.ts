export class complex
{
    /** The real component of the complex number */
    private _real: number;
    /** The imaginary component of the complex number */
    private _img: number;

    /**
     * Construct a new complex number from two real numbers
     * @param real - The real component
     * @param imaginary - The imaginary component
     * @returns Complex number constructed from given parameters
     */
    constructor(real: number, imaginary: number)
    {
        this._real = real;
        this._img = imaginary;
    }

    /**
     * Get the real component of the complex number
     * @returns The real component - this._real
     */
    get real(): number
    {
        return this._real;
    }

    /**
     * Get the imaginary component of the complex number
     * @returns The imaginary component - this._imaginary
     */
    get img(): number
    {
        return this._img;
    }

    /**
     * Add two complex numbers
     * @param other - The 2nd complex number operand
     * @returns x + other
     */
    public add(other: complex): complex
    {
        return new complex (this._real + other.real, this._img + other.img);
    }

    /**
     * Subtract two complex numbers
     * @param other - The 2nd complex number operand
     * @returns x - other
     */
    public sub(other: complex): complex
    {
        return new complex (this._real - other.real, this._img - other.img);
    }

    /**
     * Multiply two complex numbers
     * @param other - The 2nd complex number operand
     * @returns The product of x / other
     */
    public mult(other: complex): complex
    {
        return new complex (this.real * other.real - this._img * other.img,
            this.real * other.img + this._img * other.real);
    }

    /**
     * Divide two complex numbers
     * @param other - The 2nd complex number operand
     * @returns The result of the division x / other
     */
    public div(other: complex): complex
    {
        /* Complex division:
            ac + bd     bc - ad
            -------- + -------- i
            c^2 + d^2  c^2 + d^2
        */
        let ac = this._real * other.real;
        let bd = this._img * other.img;
        let bc = this._img * other.real;
        let ad = this._real * other.img;
        let cc = other.real * other.real;
        let dd = other.img * other.img;
        return new complex ((ac + bd) / (cc + dd), (bc - ad) / (cc + dd));
    }

    /**
     * Scalar multiply a complex number, by a real number lambda
     * @param lambda - The real number scaling factor
     * @returns The scaled version of the complex number
     */
    public scalarMult(lambda: number): complex
    {
        return new complex (lambda * this.real, lambda * this.img);
    }

    /**
     * Get the magnitude(absolute value) of the complex number
     * @returns The magnitude: sqroot(a^2 + b^2)
     */
    public mag(): number
    {
        return Math.sqrt((this.real * this.real) + (this.img * this.img));
    }

    /**
     * Get the conjugate of the complex number
     * @returns The conjugate of the complex number:  a + (-bi)
     */
    public conj(): complex
    {
        return new complex (this.real, -this.img);
    }

    /**
     * Get the negation of the complex number
     * @returns The negation of the complex number:  -a + (-bi)
     */
    public neg(): complex
    {
        return new complex (-this.real, -this.img);
    }

    /**
     * Get the arguement of the complex number, the angle in radians with the x-axis in polar
     * coordinates
     * @returns The arguement of the complex number
     */
    public arg(): number
    {
        return Math.atan2(this.img, this.real);
    }

    /**
     * Get the exponential of the complex number
     * @returns The exponential of the complex number: (exp(a) * cos(b)) + (exp(a) * sin(b))(i)
     */
    public exp(): complex
    {
        return new complex (
            Math.exp(this.real) * Math.cos(this.img), Math.exp(this.real) * Math.sin(this.img));
    }

    /**
     * Get the natural base e log of the complex number
     * @returns The natural base e log of the complex number
     */
    public log(): complex
    {
        return new complex (Math.log(this.mag()), Math.atan2(this.img, this.real));
    }

    /**
     * Get the sine of the complex number
     * @returns The sine of the complex number
     */
    public sin(): complex
    {
        return new complex (
            Math.cosh(this.img) * Math.sin(this.real), Math.sinh(this.img) * Math.cos(this.real));
    }

    /**
     * Get the cosine of the complex number
     * @returns The cosine of the complex number
     */
    public cos(): complex
    {
        return new complex (
            Math.cosh(this.img) * Math.cos(this.real), -Math.sinh(this.img) * Math.sin(this.real));
    }

    /**
     * Get the tangent of the complex number
     * @returns The tangent of the complex number
     */
    public tan(): complex
    {
        // defined in terms of the identity tan(z) = sin(z) / cos(z)
        let num = this.sin();
        let denom = this.cos();
        return num.div(denom);
    }

    /**
     * Static method to construct a complex number in rectangular form from polar coordinates
     * @param theta - The angle/arguement
     * @param magnitude - The magnitude
     * @returns Complex number in rectangular coordinates constructed from the arguement theta & the
     *     magnitude
     */
    public static fromPolar(theta: number, magnitude: number): complex
    {
        return new complex (magnitude * Math.cos(theta), magnitude * Math.sin(theta));
    }

    /**
     * Get the complex number's polar coordinates as a tuple
     * @returns A tuple containing the arguement/angle of the complex number as the 1st element, and
     *     the magnitude as the 2nd
     */
    public toPolar(): [number, number]
    {
        let mag = this.mag();
        let theta = this.arg();
        return [theta, mag];
    }

    /**
     * Get the complex number as a string
     * @returns String representation of the complex number
     */
    public toString(): string
    {
        if (Math.sign(this.img) === -1)
        {
            // bit of a dirty hack..
            return this.real + " - " + -this.img + "i";
        }
        else
        {
            return this.real + " + " + this.img + "i";
        }
    }

    /**
     * Compare two complex numbers for equality
     * @param other - The 2nd complex number operand
     * @returns true if equal, else false
     */
    public equals(other: complex): boolean
    {
        if (this.real === other.real && this.img === other.img)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}