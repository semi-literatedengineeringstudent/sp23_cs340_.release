from app import app
import pytest
import json, io, os

@pytest.fixture(scope='module')
def test_client():
    flask_app = app
    with flask_app.test_client() as testing_client:
        # Establish an application context
        with flask_app.app_context():
            yield testing_client

@pytest.fixture(autouse=True, scope='session')
def pytest_sessionstart():
    os.system("make clean")
    os.system("make")

def test_no_hidden_gif(test_client):
    test_file = open('sample/no-uiuc-chunk.png', 'rb')

    data = {'png': (io.BytesIO(test_file.read()), 'no-uiuc-chunk.png')}
    response = test_client.post('/extract', data=data, content_type='multipart/form-data')
    assert(response.status_code == 415)

def test_not_valid_png(test_client):
    test_file = open('sample/waf.gif', 'rb')

    data = {'png': (io.BytesIO(test_file.read()), 'waf.gif')}
    response = test_client.post('/extract', data=data, content_type='multipart/form-data')
    assert(response.status_code == 422)

def test_hidden_gif_waf(test_client):
    test_file = open('sample/waf.png', 'rb')

    data = {'png': (io.BytesIO(test_file.read()), 'waf.png')}
    response = test_client.post('/extract', data=data, content_type='multipart/form-data')
    assert(response.status_code == 200)
    assert(response.get_data() == open('sample/waf.gif','rb').read())

def test_hidden_gif_natalia(test_client):
    test_file = open('sample/natalia.png', 'rb')

    data = {'png': (io.BytesIO(test_file.read()), 'natalia.png')}
    response = test_client.post('/extract', data=data, content_type='multipart/form-data')
    assert(response.status_code == 200)
    assert(response.get_data() == open('sample/natalia.gif','rb').read())

def test_no_saved_gif(test_client):
    response = test_client.get('/extract/100')
    assert(response.status_code == 404)

def test_get_gif_waf(test_client):
    test_hidden_gif_waf(test_client)
    
    response = test_client.get('/extract/0')
    assert(response.status_code == 200)
    assert(response.get_data() == open('sample/waf.gif','rb').read())

def test_get_gif_natalia(test_client):
    test_hidden_gif_waf(test_client)
    test_hidden_gif_natalia(test_client)

    response = test_client.get('/extract/1')
    assert(response.status_code == 200)
    assert(response.get_data() == open('sample/natalia.gif','rb').read())

    
