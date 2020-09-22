import os

from flask import Flask
from flask import render_template
from flask import request

app = Flask('ROHM', static_url_path='')


@app.route("/")
def index():
    return render_template("main.html")


if __name__ == '__main__':
    port = 8080
    if 'PORT' in os.environ:
        port = os.environ['PORT']

    try:
        if os.environ['TEMPLATES_AUTO_RELOAD']:
            app.config['TEMPLATES_AUTO_RELOAD'] = True
    except KeyError:
        print('Not reloading view templates')

    app.run(port=port, host='0.0.0.0')

