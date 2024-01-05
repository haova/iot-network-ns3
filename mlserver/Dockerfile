FROM python:3.9.18

WORKDIR /root
ADD . ./mlserver

WORKDIR /root/mlserver
RUN pip install flask joblib numpy
RUN pip install -U scikit-learn

ENTRYPOINT ["python", "main.py"]