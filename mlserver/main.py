
import json
from flask import Flask, jsonify, request
import joblib
import numpy as np
app = Flask(__name__)

@app.route('/predict', methods=['POST'])
def predict():
    readings = json.loads(request.data)
    model = joblib.load("model.pkl")
    for r in readings:
        features = np.array([[r["doc"]["pdr"], r["doc"]["rss"]]])
        r["doc"]["isWarning"] = False
        prediction = model.predict(features)
        if prediction != 0:
            r["doc"]["isWarning"] = True
        
    return jsonify(readings), 200

if __name__ == '__main__':
   app.run(port=5000)