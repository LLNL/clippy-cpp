from clippy import ooclippy

ooclippy.processDirectory("/PATH/TO/clippy-cpp/build/examples/oo-dataframe")

df = Dataframe('.', "cities", [['Name', 'string'], ['Jan', 'real'], ['Apr', 'real'], ['Jul', 'real'], ['Oct', 'real'], ['avg', 'real'], ['days', 'uint'], ['snow', 'real'], ['text', 'string']])

df.importFile('/PATH/TO/clippy-cpp/examples/oo-dataframe/sample_data/city.csv')

df.metadata()

df.extreme('max', 'days')
df.extreme('min', 'days')

df.rowquery([46, 49])

