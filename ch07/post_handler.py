import sys
import urllib.parse

# 1. 표준 입력(stdin)으로부터 POST Body 데이터 읽기
# 웹 서버(부모 프로세스)가 파이프를 통해 넘겨준 데이터
post_data = sys.stdin.read()

# 2. 데이터 파싱 (name=value&msg=value 형태)
parsed_data = urllib.parse.parse_qs(post_data)

# 값 추출 (리스트 형태로 반환됨)
username = parsed_data.get('username', ['Guest'])[0]
message = parsed_data.get('msg', ['No message'])[0]

# 3. 결과 HTML 출력 (이 내용은 소켓을 타고 웹 브라우저로 전송됨)
print("<html><body>")
print("<h2>CGI Result</h2>")
print(f"<p>Raw Data: {post_data}</p>")
print("<hr>")
print(f"<p>Hello, <b>{username}</b>!</p>")
print(f"<p>Your message: <i>{message}</i></p>")
print("</body></html>")
