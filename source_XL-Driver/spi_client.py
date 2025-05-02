import socket
import threading

# Địa chỉ và cổng mà QEMU đang kết nối đến
HOST = 'localhost'
PORT = 1234
 
def listen_and_respond(sock):
    buffer = ""
    char_to_send = 'a'  # Bắt đầu với ký tự 'a'
    while True:
        try:
            # Receive data
            data = sock.recv(1024)
            if not data:
                print("Connection closed by QEMU.")
                break

            # # Log received data
           
            # print(f"Received from QEMU: {data.decode('utf-8', errors='ignore')}")
 # Chuyển dữ liệu nhận được thành chuỗi
            buffer += data.decode('utf-8', errors='ignore')
         
            # Kiểm tra và xử lý thông điệp hoàn chỉnh (dựa trên ký tự '\n')

            while '\n' in buffer:
                # Tách thông điệp đầy đủ khỏi bộ đệm
                message, buffer = buffer.split('\n', 1)
                print(f"Nhận được từ QEMU: {message.strip()}")

                # # Phản hồi lại ngay lập tức ký tự hiện tại
                # sock.sendall(char_to_send.encode('utf-8'))
                # print(f"Đã gửi: {char_to_send}")

                # Tăng ký tự để gửi ký tự tiếp theo
                # char_to_send = chr(((ord(char_to_send) - ord('a') + 1) % 26) + ord('a'))
            # Send a response
            
            response = "OK\n"
            sock.sendall(response.encode('utf-8'))
          
            # print(f"Sent to QEMU: {response.strip()}")

        except Exception as e:
            print(f"Error in communication: {e}")
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
