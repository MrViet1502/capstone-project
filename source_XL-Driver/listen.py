import socket
import threading

# Địa chỉ và cổng mà QEMU đang kết nối đến
HOST = 'localhost'
PORT = 1234
 
def listen_and_respond(sock):
    buffer = ""  # Bộ đệm để lưu dữ liệu chưa xử lý
    while True:
        try:
            # Nhận dữ liệu từ QEMU
            data = sock.recv(1024)
            if not data:
                print("Kết nối đã đóng từ QEMU.")
                break

            # Nối dữ liệu nhận được vào bộ đệm
            buffer += data.decode('utf-8', errors='ignore')

            # Xử lý thông điệp hoàn chỉnh
            while '\n' in buffer:
                # Tách thông điệp từ bộ đệm
                message, buffer = buffer.split('\n', 1)
                message = message.strip()  # Loại bỏ khoảng trắng thừa

                print(f"Nhận được từ QEMU: {message}")

                # Chỉ phản hồi nếu thông điệp hợp lệ (SPIx)
                if message.startswith("SPI") and message[3:].isdigit():
                    response = "OK\n"
                    sock.sendall(response.encode('utf-8'))
                    print(f"Đã gửi: {response.strip()}")
                
                if message.startswith("OK"):
                    response = "OK\n"
                    sock.sendall(response.encode('utf-8'))
                    print(f"Đã gửi: {response.strip()}")
                    

        except Exception as e:
            print(f"Lỗi trong giao tiếp: {e}")
            break



def main():
    # Tạo socket TCP
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            print(f"Đang kết nối tới QEMU trên {HOST}:{PORT}...")
            s.connect((HOST, PORT))
            print(f"Đã kết nối tới QEMU trên {HOST}:{PORT}!")
        except Exception as e:
            print(f"Lỗi khi kết nối: {e}")
            return

        # Lắng nghe và phản hồi dữ liệu
        listen_and_respond(s)

if __name__ == "__main__":
    main()
