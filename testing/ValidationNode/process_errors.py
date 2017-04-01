import pandas as pd

df = pd.read_csv("validation_errors.txt")
indexes_to_drop = []

#step 1 - find all rows constaining -1, remove them
df = df.query('steeringErr != -1 & brakeErr != -1 & throttleErr != -1')

#step 2 - find averages
steering_mean = df["steeringErr"].mean()
brake_mean = df["brakeErr"].mean()
throttle_mean = df["throttleErr"].mean()
print("steering MSE: {}, brake MSE: {}, throttle MSE: {}".format(steering_mean, brake_mean, throttle_mean))


