namespace brainflow.math
{
    /// <summary>
    /// Matrix major order. The default is to use C-style Row-Major order.
    /// </summary>
    ///
    public enum MatrixOrder
    {
        /// <summary>
        /// Row-major order (C, C++, C#, SAS, Pascal, NumPy default).
        /// </summary>
        CRowMajor = 1,

        /// <summary>
        /// Column-major oder (Fotran, MATLAB, R).
        /// </summary>
        /// 
        FortranColumnMajor = 0,

        /// <summary>
        /// Default (Row-Major, C/C++/C# order).
        /// </summary>
        /// 
        Default = CRowMajor
    }

    public static class MatrixHelpers
    {
        /// <summary>
        /// Gets a row vector from a matrix.
        /// </summary>
        /// 
        public static T[] GetRow<T> (this T[][] m, int index, T[] result = null)
        {
            index = MatrixHelpers.index (index, m.Rows ());

            if (result == null)
            {
                return (T[])m[index].Clone ();
            }
            else
            {
                m[index].CopyTo (result, 0);
                return result;
            }
        }

        /// <summary>
        /// Gets a row vector from a matrix.
        /// </summary>
        ///
        public static T[] GetRow<T> (this T[,] m, int index, T[] result = null)
        {
            if (result == null)
            {
                result = new T[m.GetLength (1)];
            }

            index = MatrixHelpers.index (index, m.Rows ());
            for (int i = 0; i < result.Length; i++)
            {
                result[i] = m[index, i];
            }

            return result;
        }

        /// <summary>
        /// Gets the number of rows in a vector.
        /// </summary>
        /// 
        /// <typeparam name="T">The type of the elements in the column vector.</typeparam>
        /// <param name="vector">The vector whose number of rows must be computed.</param>
        /// 
        /// <returns>The number of rows in the column vector.</returns>
        /// 
        public static int Rows<T> (this T[] vector)
        {
            return vector.Length;
        }

        /// <summary>
        /// Gets the number of rows in a multidimensional matrix.
        /// </summary>
        /// 
        /// <typeparam name="T">The type of the elements in the matrix.</typeparam>
        /// <param name="matrix">The matrix whose number of rows must be computed.</param>
        /// 
        /// <returns>The number of rows in the matrix.</returns>
        /// 
        public static int Rows<T> (this T[,] matrix)
        {
            return matrix.GetLength (0);
        }

        /// <summary>
        /// Gets the number of columns in a multidimensional matrix.
        /// </summary>
        /// 
        /// <typeparam name="T">The type of the elements in the matrix.</typeparam>
        /// <param name="matrix">The matrix whose number of columns must be computed.</param>
        /// 
        /// <returns>The number of columns in the matrix.</returns>
        /// 
        public static int Columns<T> (this T[,] matrix)
        {
            return matrix.GetLength (1);
        }

        /// <summary>
        /// Gets the number of rows in a multidimensional matrix.
        /// </summary>
        /// 
        /// <typeparam name="T">The type of the elements in the matrix.</typeparam>
        /// <param name="matrix">The matrix whose number of rows must be computed.</param>
        /// 
        /// <returns>The number of rows in the matrix.</returns>
        /// 
        public static int Rows<T> (this T[,,] matrix)
        {
            return matrix.GetLength (0);
        }

        /// <summary>
        /// Transforms a matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="matrix">A matrix.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Flatten<T> (this T[,] matrix, MatrixOrder order = MatrixOrder.Default)
        {
            return Reshape (matrix, order);
        }

        /// <summary>
        /// Transforms a matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="matrix">A matrix.</param>
        /// <param name="result">The vector where to store the copy.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Flatten<T> (this T[,] matrix, T[] result, MatrixOrder order = MatrixOrder.Default)
        {
            return Reshape (matrix, result, order);
        }


        /// <summary>
        /// Transforms a jagged array matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="array">A jagged array.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Flatten<T> (this T[][] array, MatrixOrder order = MatrixOrder.Default)
        {
            return Reshape (array, order);
        }

        /// <summary>
        /// Transforms a jagged array matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="array">A jagged array.</param>
        /// <param name="result">The vector where to store the copy.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Flatten<T> (this T[][] array, T[] result, MatrixOrder order = MatrixOrder.Default)
        {
            return Reshape (array, result, order);
        }

        /// <summary>
        /// Transforms a matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="matrix">A matrix.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Reshape<T> (this T[,] matrix, MatrixOrder order = MatrixOrder.Default)
        {
            int rows = matrix.GetLength (0);
            int cols = matrix.GetLength (1);
            return Reshape (matrix, new T[rows * cols], order);
        }

        /// <summary>
        /// Transforms a matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="matrix">A matrix.</param>
        /// <param name="result">The vector where to store the copy.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Reshape<T> (this T[,] matrix, T[] result, MatrixOrder order = MatrixOrder.Default)
        {
            int rows = matrix.GetLength (0);
            int cols = matrix.GetLength (1);

            if (order == MatrixOrder.CRowMajor)
            {
                int k = 0;
                for (int j = 0; j < rows; j++)
                    for (int i = 0; i < cols; i++)
                        result[k++] = matrix[j, i];
            }
            else
            {
                int k = 0;
                for (int i = 0; i < cols; i++)
                    for (int j = 0; j < rows; j++)
                        result[k++] = matrix[j, i];
            }

            return result;
        }


        /// <summary>
        /// Transforms a jagged array matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="array">A jagged array.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Reshape<T> (this T[][] array, MatrixOrder order = MatrixOrder.Default)
        {
            int count = 0;
            for (int i = 0; i < array.Length; i++)
                count += array[i].Length;
            return Reshape (array, new T[count], order);
        }

        /// <summary>
        ///   Transforms a jagged array matrix into a single vector.
        /// </summary>
        /// 
        /// <param name="array">A jagged array.</param>
        /// <param name="result">The vector where to store the copy.</param>
        /// <param name="order">The direction to perform copying. Pass
        ///   1 to perform a copy by reading the matrix in row-major order.
        ///   Pass 0 to perform a copy in column-major copy. Default is 1 
        ///   (row-major, c-style order).</param>
        /// 
        public static T[] Reshape<T> (this T[][] array, T[] result, MatrixOrder order = MatrixOrder.Default)
        {
            if (order == MatrixOrder.CRowMajor)
            {
                int k = 0;
                for (int j = 0; j < array.Length; j++)
                    for (int i = 0; i < array[j].Length; i++)
                        result[k++] = array[j][i];
            }
            else
            {
                int maxCols = 0;
                for (int i = 0; i < array.Length; i++)
                {
                    if (array[i].Length > maxCols)
                        maxCols = array[i].Length;
                }

                for (int i = 0, k = 0; i < maxCols; i++)
                {
                    for (int j = 0; j < array.Length; j++)
                    {
                        if (i < array[j].Length)
                            result[k++] = array[j][i];
                    }
                }
            }

            return result;
        }

        /// <summary>
        /// Combines two vectors horizontally.
        /// </summary>
        /// 
        public static T[] Concatenate<T> (this T[] a, params T[] b)
        {
            T[] r = new T[a.Length + b.Length];
            for (int i = 0; i < a.Length; i++)
                r[i] = a[i];
            for (int i = 0; i < b.Length; i++)
                r[i + a.Length] = b[i];

            return r;
        }

        /// <summary>
        /// Combines a vector and a element horizontally.
        /// </summary>
        /// 
        public static T[] Concatenate<T> (this T[] vector, T element)
        {
            T[] r = new T[vector.Length + 1];
            for (int i = 0; i < vector.Length; i++)
                r[i] = vector[i];

            r[vector.Length] = element;

            return r;
        }

        /// <summary>
        /// Combines a vector and a element horizontally.
        /// </summary>
        /// 
        public static T[] Concatenate<T> (this T element, T[] vector)
        {
            T[] r = new T[vector.Length + 1];

            r[0] = element;

            for (int i = 0; i < vector.Length; i++)
                r[i + 1] = vector[i];

            return r;
        }

        /// <summary>
        /// Combines two matrices horizontally.
        /// </summary>
        /// 
        public static T[,] Concatenate<T> (this T[,] a, T[,] b)
        {
            return Concatenate (new[] { a, b });
        }

        /// <summary>
        /// Combines two matrices horizontally.
        /// </summary>
        /// 
        public static T[][] Concatenate<T> (this T[][] a, T[][] b)
        {
            return Concatenate (new[] { a, b });
        }

        /// <summary>
        /// Combines a matrix and a vector horizontally.
        /// </summary>
        /// 
        public static T[,] Concatenate<T> (params T[][,] matrices)
        {
            int rows = 0;
            int cols = 0;

            for (int i = 0; i < matrices.Length; i++)
            {
                cols += matrices[i].GetLength (1);
                if (matrices[i].GetLength (0) > rows)
                    rows = matrices[i].GetLength (0);
            }

            T[,] r = new T[rows, cols];


            int c = 0;
            for (int k = 0; k < matrices.Length; k++)
            {
                int currentRows = matrices[k].GetLength (0);
                int currentCols = matrices[k].GetLength (1);

                for (int j = 0; j < currentCols; j++)
                {
                    for (int i = 0; i < currentRows; i++)
                    {
                        r[i, c] = matrices[k][i, j];
                    }
                    c++;
                }
            }

            return r;
        }

        /// <summary>
        /// Combines a matrix and a vector horizontally.
        /// </summary>
        /// 
        public static T[][] Concatenate<T> (params T[][][] matrices)
        {
            int rows = 0;
            int cols = 0;

            for (int i = 0; i < matrices.Length; i++)
            {
                cols += matrices[i][0].Length;
                if (matrices[i].Length > rows)
                    rows = matrices[i].Length;
            }

            T[][] r = new T[rows][];
            for (int i = 0; i < r.Length; i++)
                r[i] = new T[cols];


            int c = 0;
            for (int k = 0; k < matrices.Length; k++)
            {
                int currentRows = matrices[k].Length;
                int currentCols = matrices[k][0].Length;

                for (int j = 0; j < currentCols; j++)
                {
                    for (int i = 0; i < currentRows; i++)
                    {
                        r[i][c] = matrices[k][i][j];
                    }
                    c++;
                }
            }

            return r;
        }

        /// <summary>
        /// Combine vectors horizontally.
        /// </summary>
        /// 
        public static T[] Concatenate<T> (this T[][] vectors)
        {
            int size = 0;
            for (int i = 0; i < vectors.Length; i++)
                size += vectors[i].Length;

            T[] r = new T[size];

            int c = 0;
            for (int i = 0; i < vectors.Length; i++)
                for (int j = 0; j < vectors[i].Length; j++)
                    r[c++] = vectors[i][j];

            return r;
        }

        private static int index (int end, int length)
        {
            if (end < 0)
            {
                end = length + end;
            }
            return end;
        }
    }
}