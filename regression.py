from sklearn.preprocessing import PolynomialFeatures
from sklearn.base import MultiOutputMixin, RegressorMixin, BaseEstimator
from sklearn.linear_model import Ridge, LinearRegression, Lasso, BayesianRidge
from sklearn.pipeline import Pipeline
from sklearn.model_selection import LeaveOneOut, cross_validate, GridSearchCV
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

MAKE_VALIDATION = True
PREDICT_R = True

class RaxisRegressor(MultiOutputMixin, RegressorMixin, BaseEstimator):
    def __init__(self):
        estimator = Pipeline([('polyfeats', PolynomialFeatures(3, include_bias=False)),
                              ('reg', BayesianRidge(n_iter=400,tol=1e-4))])
        self.estimator = GridSearchCV(estimator, cv=LeaveOneOut(), scoring='neg_mean_absolute_error',
                                      param_grid={'polyfeats__degree': [1, 2, 3]})

    def fit(self, X, y):
        """
        Args:
            X: dataframe with column 'x_axis' and column 'y_axis'.
        """
        Xr = np.linalg.norm(X[['x_axis', 'y_axis']]+1, axis=1)
        speed_r = np.linalg.norm(y, axis=1)
        self.estimator.fit(Xr.reshape(-1, 1), speed_r)
        # print(self.linearReg.best_estimator_['polyfeats'].degree)

        return self

    def predict(self, X):
        if('x_axis' in X.columns):
            axis = X[['x_axis', 'y_axis']] + 1
            axis_r = np.linalg.norm(axis, axis=1)
        else:
            axis_r = X['r_axis'].values
        speed_r = self.estimator.predict(axis_r.reshape(-1, 1))
        if(not ('x_axis' in X.columns)):
            return speed_r
        speed_x = speed_r * axis['x_axis']/axis_r
        speed_y = speed_r * axis['y_axis']/axis_r
        return np.vstack((speed_x, speed_y)).T

    def getBestCoeficients(self):
        return self.estimator.best_estimator_['reg'].coef_

    def getBestDegree(self):
        return self.estimator.best_params_['polyfeats__degree']


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True,
                        help="csv file.")
    parser.add_argument('-o', '--output', type=str, required=True,
                        help="csv file.")
    args = parser.parse_args()
    
    data = pd.read_csv(args.input)
    data = data.sort_values('x_axis')
    rareg = RaxisRegressor()
    if(not ('y_speed' in data)):
        assert((data['y_axis'] == 0).all()), "No y_speed provided. Cannot assume is 0 if not all y_axis are 0."
        data['y_speed'] = 0
    print(data)
    target = data[['x_speed', 'y_speed']]
    rareg.fit(data, target)

    if(PREDICT_R):
        #axis_test = np.linalg.norm(data[['x_axis','y_axis']]+1, axis=1)
        axis_test = np.arange(0, 128)
        axis_test = pd.DataFrame(axis_test.reshape(-1,1), columns=['r_axis'])
        axis_test['pred_r'] = rareg.predict(axis_test)
        plt.plot(axis_test['r_axis'],axis_test['pred_r'])
        Xr = np.linalg.norm(data[['x_axis', 'y_axis']]+1, axis=1)
        plt.plot(Xr,data['x_speed'], marker='o')
        plt.savefig('regression.png')
        plt.show()
        axis_test.to_csv(args.output, sep=';', columns=['r_axis', 'pred_r'], header=False, index=False)
    else:
        axis_test = np.zeros((128, 2), dtype=np.int)
        axis_test[:, 0] = np.arange(0, 128)
        axis_test = pd.DataFrame(axis_test, columns=['x_axis', 'y_axis'])
        preds = rareg.predict(axis_test)
        axis_test['pred_x'] = preds[:, 0]
        axis_test['pred_y'] = preds[:, 1]
        plt.plot(axis_test['x_axis'],axis_test['pred_x'])
        plt.plot(data['x_axis'],data['x_speed'], marker='o')
        plt.show()
        axis_test['x_axis']+=1
        axis_test.to_csv(args.output, sep=';', columns=['x_axis', 'pred_x'], header=False, index=False)

    #negative_velocities = axis_test[axis_test['pred_x'] < 0]
    #if(len(negative_velocities) > 0):
    #    deadzone = negative_velocities['x_axis'].max()
    #    axis_test = axis_test[axis_test['x_axis'] > deadzone]

    #print(axis_test)
    


    if(MAKE_VALIDATION):
        scores = cross_validate(rareg, data, target, cv=LeaveOneOut(),
                                scoring='neg_mean_absolute_error', return_estimator=True)
        print(-scores['test_score'], -scores['test_score'].mean())
        print([estimator.getBestDegree() for estimator in scores['estimator']])
