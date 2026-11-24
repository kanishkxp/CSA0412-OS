"""Small stand-in for the university result portal (Flask)."""
import os
import hashlib
from flask import Flask, jsonify

app = Flask(__name__)


@app.route("/")
def home():
    return "SIMATS Result Portal is running\n"


@app.route("/health")
def health():
    return jsonify(status="ok")


@app.route("/result/<regno>")
def result(regno):
    # small amount of CPU work so that the load test is not completely empty
    digest = hashlib.sha256(regno.encode()).hexdigest()
    marks = int(digest[:4], 16) % 41 + 60
    return jsonify(register_number=regno, marks=marks, host=os.uname().nodename)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
