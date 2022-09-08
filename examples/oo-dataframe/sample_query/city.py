from clippy import ooclippy

ooclippy.processDirectory("/PATH/TO/clippy-cpp/build/examples/oo-dataframe")

# create a new dataframe by defining the columns
df = Dataframe('.', "cities", columns = [['Name', 'string'], ['Jan', 'real'], ['Apr', 'real'], ['Jul', 'real'], ['Oct', 'real'], ['avg', 'real'], ['days', 'uint'], ['snow', 'real'], ['text', 'string']])

# import sample CSV
df.importFile('/PATH/TO/clippy-cpp/examples/oo-dataframe/sample_data/city.csv')

# or for attaching to an existing dataframe, one can use:
# df = Dataframe('.', "cities")

df.metadata()

df.extreme('max', 'days')
df.extreme('min', 'days')

df.rowquery([46, 49])

