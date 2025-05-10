

from flask import Flask
from datetime import datetime

app = Flask(__name__)

@app.route("/datetime")
def get_datetime():
    now = datetime.now()
    return now.strftime("%Y-%m-%d %H:%M:%S")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)