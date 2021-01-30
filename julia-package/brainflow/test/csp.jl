using BrainFlow

data = Array{Float64,3}(undef, 2, 4, 2)
data[:, :, 1] = [[6,3] [3,0] [1,5] [5,1]]
data[:, :, 2] = [[1,5] [5,1] [6,2] [2,2]]
labels = [0.0, 1.0]

filters, eigvals = BrainFlow.get_csp(data, labels)

println("filters = ")
println(filters)
