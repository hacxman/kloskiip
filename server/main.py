import BaseHTTPServer

class Server(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_GET(self):
    data = {}
    data['ok'] = 'true'
    self.send_response(200, 'OK')

    if self.path == '/':
      data = open('index.html').read()

    self.end_headers()
    self.wfile.write(str(data))
    print self.path, str(data)

def run(server_class=BaseHTTPServer.HTTPServer,
        handler_class=BaseHTTPServer.BaseHTTPRequestHandler):
    server_address = ('', 8000)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()

run(handler_class=Server)
