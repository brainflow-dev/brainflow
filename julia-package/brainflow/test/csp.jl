using BrainFlow
data = Array{Float64,3}(undef, 2, 4, 2)
data[:, :, 1] = [[6,3] [3,0] [1,5] [5,1]]
data[:, :, 2] = [[1,5] [5,1] [6,2] [2,2]]
data
labels = [0, 1]
BrainFlow.get_csp(data, labels)


n_epochs = size(data, 3)
n_channels = size(data, 1)
n_times = size(data, 2)

temp_data1d = Vector{Float64}(undef, Integer(n_epochs * n_channels * n_times))
for e=1:n_epochs
    for c=1:n_channels
        for t=1:n_times
            temp_data1d[(e-1) * n_channels * n_times + (c-1) * n_times + t] = data[c, t, e]
        end
    end
end

temp_filters = Vector{Float64}(undef, Integer(n_channels * n_channels))
output_eigenvalues = Vector{Float64}(undef, Integer(n_channels))

ccall((:get_csp, DATA_HANDLER_INTERFACE), Cint, (Ptr{Float64}, Ptr{Float64}, Cint, Cint, Cint, Ptr{Float64}, Ptr{Float64}), data, labels, Int32(n_epochs), Int32(n_channels), Int32(n_times), temp_filters, output_eigenvalues)
