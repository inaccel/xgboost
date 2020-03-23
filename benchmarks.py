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
    params = {'max_depth':args.depth, 'verbosity':args.verbosity, 'eta':'0.1'}
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
    bst = xgb.train(params, dtrain, args.rounds,[(dtrain, "train"), (dtest, "val")])
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

def SVHN_dataset():
    print("Loading SVHN")
    X, y = datasets.load_svmlight_file("data/SVHN.bz2")
    X_train = X[0:65000, : ]
    y_train = y[0:65000]
    X_test, y_test = datasets.load_svmlight_file("data/SVHN.t.bz2")
    return X_train, y_train, X_test, y_test

def synthetic_regression_dataset(nfeatures):
    print("Creating Synthetic Regression")
    X, y = datasets.make_regression(n_samples=81250, n_features=nfeatures, bias=100, noise=1.0, random_state=random_seed)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def synthetic_classification_dataset(nfeatures):
    print("Creating Synthetic Multiclass classification")
    X, y = datasets.make_classification(n_samples=81250, n_features=nfeatures, n_informative=5, n_redundant=5, n_repeated=1, n_classes=5)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=random_seed)
    return X_train, y_train, X_test, y_test

def main():
    datasets = "Cifar10,SVHN,SyntheticR,SyntheticCl"
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r','--rounds', type=int, default=5, help='Boosting rounds.')
    parser.add_argument('-d','--datasets', default=datasets, help='Datasets to run. Must be included in the Default')
    parser.add_argument('-v','--verbosity', type=int, default=0, help='XGBoost verbosity parameter.')
    parser.add_argument('-t','--nthreads', type=int, default=None, help='Number of threads to use.')
    parser.add_argument('-R','--nrequests', type=int, default=4, help='Number of requests for Coral manager. Not used with the standalone version')
    parser.add_argument('-f','--nfeatures', type=int, nargs='+', default=1024, help='Number of features for the synthetic datasets')
    parser.add_argument('-D','--depth', type=int, default=10, help='The maximum depth of the tree')
    args = parser.parse_args()

    columns = ['Time(s)','Accuracy','RMSE','SpeedUp']
    df = pd.DataFrame()

    if "Cifar10" in args.datasets:
        X_train, y_train, X_test, y_test = cifar10_dataset()
        iterables = [["Cifar10"],['cpu','fpga']]
        index = pd.MultiIndex.from_product(iterables)
        df2 = pd.DataFrame( index=index, columns=columns)
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "Cifar10", "Multiclass classification", "Accuracy", df2, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "Cifar10", "Multiclass classification", "Accuracy", df2, args)
        calc_speedup(df2, "Cifar10")
        df = df.append(df2)
        print(df.to_string())

    if "SVHN" in args.datasets:
        X_train, y_train, X_test, y_test = SVHN_dataset()
        iterables = [["SVHN"],['cpu','fpga']]
        index = pd.MultiIndex.from_product(iterables)
        df2 = pd.DataFrame( index=index, columns=columns)
        train_xgboost(X_train, y_train, X_test, y_test,'fpga', "SVHN", "Multiclass classification", "Accuracy", df2, args)
        train_xgboost(X_train, y_train, X_test, y_test,'cpu', "SVHN", "Multiclass classification", "Accuracy", df2, args)
        calc_speedup(df2, "SVHN")
        df = df.append(df2)
        print(df.to_string())
                   
    if "SyntheticR" in args.datasets:
        for nfeat in args.nfeatures:
            X_train, y_train, X_test, y_test = synthetic_regression_dataset(nfeat)
            iterables = [["SyntheticR "+str(nfeat)],['cpu','fpga']]
            index = pd.MultiIndex.from_product(iterables)
            df2 = pd.DataFrame( index=index, columns=columns)
            train_xgboost(X_train, y_train, X_test, y_test,'cpu', "SyntheticR "+str(nfeat), "Regression", "RMSE", df2, args)
            train_xgboost(X_train, y_train, X_test, y_test,'fpga', "SyntheticR "+str(nfeat), "Regression", "RMSE", df2, args)
            calc_speedup(df2, "SyntheticR "+str(nfeat))
            df = df.append(df2)
            print(df.to_string())

    if "SyntheticCl" in args.datasets:
        for nfeat in args.nfeatures:
            X_train, y_train, X_test, y_test = synthetic_classification_dataset(nfeat)
            iterables = [["SyntheticCl "+str(nfeat)],['cpu','fpga']]
            index = pd.MultiIndex.from_product(iterables)
            df2 = pd.DataFrame( index=index, columns=columns)
            train_xgboost(X_train, y_train, X_test, y_test,'fpga', "SyntheticCl "+str(nfeat), "Multiclass classification", "Accuracy", df2, args)
            train_xgboost(X_train, y_train, X_test, y_test,'cpu', "SyntheticCl "+str(nfeat), "Multiclass classification", "Accuracy", df2, args)
            calc_speedup(df2, "SyntheticCl "+str(nfeat))
            df = df.append(df2)
            print(df.to_string())
    
if __name__ == "__main__":
    main()
