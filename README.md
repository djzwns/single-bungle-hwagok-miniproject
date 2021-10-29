서울기술교육센터 프로젝트
==============
Smart Box 
-------------
0. 시연 영상
	- <img src="https://user-images.githubusercontent.com/17754057/139484092-28afbdbe-953c-4747-bb71-ae67fef6a896.gif">
	- 운송장 번호(적외선리모컨 활용) 입력 시 택배함 잠금
	- 서버에 등록되지 않은 카드 / 서버에 등록된 카드에 따른 LED 점멸 및 택배함 잠금해제
	
2. smartbox-client
	- 아두이노를 활용한 클라이언트 코드
	- 센서 코드 분할
	- 서버로 데이터 송수신하여 센서 동작

2. smartbox-server
	- mariadb 를 통해 간단한 데이터 관리
		* 운송장 번호와 카드번호 등록 및 조회
