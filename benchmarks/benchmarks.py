import argparse
import sys
import time

import numpy as np
import pandas as pd
import xgboost as xgb
from sklearn import datasets
from sklearn.metrics import mean_squared_error, accuracy_score
from sklearn.model_selection import train_test_split

# Global parameters
random_seed = 0
max_depth = 10
learning_rate = 0.1
min_split_loss = 0
min_weight = 1
l1_reg = 0
l2_reg = 1

def eval(task, metric_t, y_test, pred):
    if metric_t == "RMSE":
        return np.sqrt(mean_squared_error(y_test, pred))
    elif metric_t == "Accuracy":
        # Threshold prediction if binary classification
        if task == "Classification":
            pred = pred > 0.5
        elif task == "Multiclass classification":
            if pred.ndim > 1:
                pred = np.argmax(pred, axis=1)
        return accuracy_score(y_test, pred)
    else:
        raise ValueError("Unknown metric: " + data.metric)

def add_data(df, algorithm, name, metric_t, elapsed, metric):
    df.loc[(name,algorithm), 'Time(s)'] = elapsed
    df.loc[(name,algorithm), metric_t] = metric

def calc_speedup(df, name):
    df.loc[(name,'fpga'), 'SpeedUp'] = df.loc[(name,'cpu'), 'Time(s)']/df.loc[(name,'fpga'), 'Time(s)']

def configure_xgboost(nclass, alg, task, args):
    params = {'max_depth':max_depth, 'verbosity':args.verbosity, 'eta':'0.1'}
    if alg == 'cpu':
        params['tree_method'] = 'exact'
    elif alg == 'fpga':
        params['tree_method'] = 'fpga_exact'
    else:
        raise ValueError("Unknown Updater: " + alg)

    if args.nthreads != None:
        params['nthread'] = args.nthreads
    if args.nrequests != None:
        params['nRequests'] = args.nrequests
    
    if task == "Regression":
        params["objective"] = "reg:squarederror"
    elif task == "Multiclass classification":
        params["objective"] = "multi:softmax"
        params["num_class"] = int(nclass)
    elif task == "Classification":
        params["objective"] = "binary:logistic"
    else:
        raise ValueError("Unknown task: " + task)
    return params

def run_xgboost(task, metric_t, dtrain, dtest, y_test, params, args):
    start = time.time()
    bst = xgb.train(params, dtrain, args.num_rounds,[(dtrain, "train"), (dtest, "val")])
    elapsed = time.time() - start
    pred = bst.predict(dtest)
    metric = eval(task, metric_t, y_test, pred)
    return elapsed, metric

def train_xgboost(X_train, y_train, X_test, y_test, alg, name, task, metric_t, df, args, ncores=None):
    print("Starting training "+name+" "+alg)
    dtrain = xgb.DMatrix(X_train, y_train)
    dtest = xgb.DMatrix(X_test, y_test)
    nclass = np.max(y_test) + 1
    params = configure_xgboost(nclass, alg, task, args)
    elapsed, metric = run_xgboost(task, metric_t, dtrain, dtest, y_test, params, args)
    add_data(df, alg, name, metric_t, elapsed, metric)
    df.fillna('',inplace=True)

def cifar10_dataset():
    print("Loading Cifar10")
    return datasets.load_svmlight_files(("data/cifar10.bz2", "data/cifar10.t.bz2"))

def mnist_dataset():
    print("Loading MNIST")
    return datasets.load_svmlight_files(("data/mnist.bz2", "data/mnist.t.bz2"))

def news20_dataset():
    print("Loading News20")
    return datasets.load_svmlight_files(("data/news20.bz2", "data/news20.t.bz2"))

def higgs_dataset():
    print("Loading Higgs")
    X, y = datasets.load_svmlight_file('data/HIGGS.bz2')
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, train_size=60000, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def year_dataset():
    print("Loading YearPredictionMSD")
    X, y = datasets.load_svmlight_file('data/YearPredictionMSD.bz2')
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, train_size=60000, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def cover_type_dataset():
    print("Loading Cover Type")
    X, y = datasets.load_svmlight_file('data/covtype.libsvm.binary.bz2')
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, train_size=60000, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def airline_dataset():
    print("Loading Airline")
    cols = [
        "Year", "Month", "DayofMonth", "DayofWeek", "CRSDepTime",
        "CRSArrTime", "UniqueCarrier", "FlightNum", "ActualElapsedTime",
        "Origin", "Dest", "Distance", "Diverted", "ArrDelay"
    ]
    dtype = np.int16
    dtype_columns = {
        "Year": dtype, "Month": dtype, "DayofMonth": dtype, "DayofWeek": dtype,
        "CRSDepTime": dtype, "CRSArrTime": dtype, "FlightNum": dtype,
        "ActualElapsedTime": dtype, "Distance":
            dtype,
        "Diverted": dtype, "ArrDelay": dtype,
    }
    df = pd.read_csv("data/airline_14col.data.bz2", names=cols, dtype=dtype_columns, nrows=75000)
    for col in df.select_dtypes(['object']).columns:
        df[col] = df[col].astype("category").cat.codes
    df["ArrDelayBinary"] = 1 * (df["ArrDelay"] > 0)
    X = df[df.columns.difference(["ArrDelay", "ArrDelayBinary"])]
    y = df["ArrDelayBinary"]
    del df
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def synthetic_regression_dataset(nfeatures):
    print("Creating Synthetic Regression")
    X, y = datasets.make_regression(n_samples=75000, n_features=nfeatures, bias=100, noise=1.0, random_state=random_seed)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def synthetic_classification_dataset(nfeatures):
    print("Creating Synthetic Multiclass classification")
    X, y = datasets.make_classification(n_samples=75000, n_features=nfeatures, n_informative=5, n_redundant=5, n_repeated=1, n_classes=5)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def main():
    datasets = "Cifar10,MNIST,Higgs,YearPredictionMSD,Cover Type,Airline,SyntheticR,SyntheticCl"
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--num_rounds', type=int, default=5, help='Boosting rounds.')
    parser.add_argument('--datasets', default=datasets, help='Datasets to run. Must be included in the Default')
    parser.add_argument('--verbosity', type=int, default=0, help='XGBoost verbosity parameter.')
    parser.add_argument('--nthreads', type=int, default=None, help='Number of threads to use.')
    parser.add_argument('--nrequests', type=int, default=4, help='Number of requests for Coral manager. Not used with the standalone version')
    parser.add_argument('--nfeatures', type=int, default=1000, help='Number of features for the synthetic datasets')
    args = parser.parse_args()

    iterables = [args.datasets.split(','),['cpu','fpga']]
    index = pd.MultiIndex.from_product(iterables)
    columns = ['Time(s)','Accuracy','RMSE','SpeedUp']
    df = pd.DataFrame( index=index, columns=columns)

    if "Cifar10" in args.datasets:
        X_train, y_train, X_test, y_test = cifar10_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "Cifar10", "Multiclass classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "Cifar10", "Multiclass classification", "Accuracy", df, args)
        calc_speedup(df, "Cifar10")
        print(df.to_string())
   
    if "MNIST" in args.datasets:
        X_train, y_train, X_test, y_test = mnist_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "MNIST", "Multiclass classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "MNIST", "Multiclass classification", "Accuracy", df, args)
        calc_speedup(df, "MNIST")
        print(df.to_string())

    if "News20" in args.datasets:
        X_train, y_train, X_test, y_test = news20_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "News20", "Multiclass classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "News20", "Multiclass classification", "Accuracy", df, args)
        calc_speedup(df, "News20")
        print(df.to_string())
    
    if "Higgs" in args.datasets:
        X_train, y_train, X_test, y_test = higgs_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "Higgs", "Classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "Higgs", "Classification", "Accuracy", df, args)
        calc_speedup(df, "Higgs")
        print(df.to_string())
    
    if "YearPredictionMSD" in args.datasets:
        X_train, y_train, X_test, y_test = year_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "YearPredictionMSD", "Regression", "RMSE", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "YearPredictionMSD", "Regression", "RMSE", df, args)
        calc_speedup(df, "YearPredictionMSD")
        print(df.to_string())
    
    if "Cover Type" in args.datasets:
        X_train, y_train, X_test, y_test = cover_type_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "Cover Type", "Multiclass classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "Cover Type", "Multiclass classification", "Accuracy", df, args)
        calc_speedup(df, "Cover Type")
        print(df.to_string())
    
    if "Airline" in args.datasets:
        X_train, y_train, X_test, y_test = airline_dataset()
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "Airline", "Classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "Airline", "Classification", "Accuracy", df, args)
        calc_speedup(df, "Airline")
        print(df.to_string())
    
    if "SyntheticR" in args.datasets:
        X_train, y_train, X_test, y_test = synthetic_regression_dataset(args.nfeatures)
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "SyntheticR", "Regression", "RMSE", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "SyntheticR", "Regression", "RMSE", df, args)
        calc_speedup(df, "SyntheticR")
        print(df.to_string())

    if "SyntheticCl" in args.datasets:
        X_train, y_train, X_test, y_test = synthetic_classification_dataset(args.nfeatures)
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "SyntheticCl", "Multiclass classification", "Accuracy", df, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "SyntheticCl", "Multiclass classification", "Accuracy", df, args)
        calc_speedup(df, "SyntheticCl")
        print(df.to_string())
    
if __name__ == "__main__":
    main()
