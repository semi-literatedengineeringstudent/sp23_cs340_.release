import json
from app import app
import pytest
import subprocess
import os
import dotenv
import time
import sys
import math

@pytest.fixture(autouse=True, scope='session')
def pytest_sessionstart():
    dotenv.load_dotenv()
    yield

@pytest.fixture(scope='module')
def start_microservice():
    host, port = os.getenv('MANDELBROT_MICROSERVICE_URL').split("/")[2].split(":")

    microservice = subprocess.Popen([sys.executable, "-m", "flask", "run", "--host", host, "--port", port], cwd="mandelbrot_microservice")
    time.sleep(0.5)
    yield
    microservice.terminate()


#
# This is done via the GitHub Action
#
# @pytest.fixture(scope='module')
# def require_docker():
#     proc_run = subprocess.Popen(os.environ['DOCKER_RUN_COMMAND'].split())
#     proc_run.wait()
#     assert(proc_run.returncode == 0)

#     yield

#     proc_stop = subprocess.Popen(os.environ['DOCKER_STOP_COMMAND'].split())
#     proc_stop.wait()
#     assert(proc_stop.returncode == 0)


@pytest.fixture(scope='module')
def test_client():
    # ensure the docker container is running
    flask_app = app
    with flask_app.test_client() as testing_client:
        # Establish an application context
        with flask_app.app_context():
            yield testing_client

INITIAL_STATE = {'colormap':'cividis', 'real':-0.7435, 'imag':0.126129, 'height':0.00018972901232843951, 'dim':256, 'iter':512}

def test_move_up(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/moveUp')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["imag"], INITIAL_STATE["imag"] + (INITIAL_STATE['height'] / 4)))


def test_move_down(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/moveDown')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["imag"], INITIAL_STATE["imag"] - (INITIAL_STATE['height'] / 4)))


def test_move_right(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/moveRight')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["real"], INITIAL_STATE["real"] + (INITIAL_STATE['height'] / 4)))


def test_move_left(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/moveLeft')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["real"], INITIAL_STATE["real"] - (INITIAL_STATE['height'] / 4)))

def test_zoom_in(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/zoomIn')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["height"], INITIAL_STATE["height"] * (1/1.4)))

def test_zoom_out(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/zoomOut')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(math.isclose(r.json["height"], INITIAL_STATE["height"] * (1.4)))

def test_larger_image(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/largerImage')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(
        math.isclose(r.json["dim"], INITIAL_STATE["dim"] * (1.25))
        or 
        math.isclose(r.json["dim"], round(INITIAL_STATE["dim"] * (1.25)))
    )

def test_smaller_image(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/smallerImage')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(
        math.isclose(r.json["dim"], INITIAL_STATE["dim"] * (1 / 1.25))
        or 
        math.isclose(r.json["dim"], round(INITIAL_STATE["dim"] * (1 / 1.25)))
    )

def test_more_iterations(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/moreIterations')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(
        math.isclose(r.json["iter"], INITIAL_STATE["iter"] * 2)
        or
        math.isclose(r.json["iter"], round(INITIAL_STATE["iter"] * 2))
    )

def test_less_iterations(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/lessIterations')
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(
        math.isclose(r.json["iter"], INITIAL_STATE["iter"] / 2)
        or
        math.isclose(r.json["iter"], round(INITIAL_STATE["iter"] / 2))
    )

def test_change_colormap(test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)

    r = test_client.post('/changeColorMap', json={'colormap':'plasma'})
    assert(r.status_code == 200)

    r = test_client.get('/getState')
    assert(r.status_code == 200)
    assert(r.json["colormap"] == "plasma")


def test_cache(start_microservice, test_client):
    r = test_client.get('/clearCache')

    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    r = test_client.post('/lessIterations')
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    r = test_client.post('/lessIterations')
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    r = test_client.post('/lessIterations')
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    r = test_client.post('/moreIterations')
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    r = test_client.post('/moveLeft')
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)


    r = test_client.get('/storage')
    assert(r.status_code == 200)

    # 5 images should be in the cache
    assert(len(r.get_json()) == 5)


def test_cache_replay(start_microservice, test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    r = test_client.get('/mandelbrot')

    r = test_client.post('/lessIterations')
    r = test_client.get('/mandelbrot')

    r = test_client.post('/lessIterations')
    r = test_client.get('/mandelbrot')

    r = test_client.post('/lessIterations')
    r = test_client.get('/mandelbrot')

    r = test_client.post('/moreIterations')
    r = test_client.get('/mandelbrot')

    r = test_client.post('/moveLeft')
    r = test_client.get('/mandelbrot')

    r = test_client.post('/moveLeft')
    r = test_client.get('/mandelbrot')

    r = test_client.get('/storage')

    # 5 old + 1 new images should be in the cache
    assert(len(r.get_json()) == 6)

def test_cache_restart(start_microservice, test_client):
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    r = test_client.get('/mandelbrot')

    r = test_client.get('/storage')

    # 6 images should still be in the cache
    assert(len(r.get_json()) == 6)    

def test_cache_x_clear(start_microservice, test_client):
    r = test_client.get('/clearCache')
    
    r = test_client.post('/resetTo', json=INITIAL_STATE)
    assert(r.status_code == 200)
    r = test_client.get('/mandelbrot')
    assert(r.status_code == 200)

    # cache is reset, and only 1 image
    r = test_client.get('/storage')
    assert(len(r.get_json()) == 1)
