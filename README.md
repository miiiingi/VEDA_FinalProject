<h1 align="center"> CAN-based_Door_Lock_Control </h1>
<h3 align="center"> CAN 통신을 활용한 실시간 출입 통제 시스템  </h3>  

<br>

<!-- TABLE OF CONTENTS -->
<h2 id="table-of-contents"> :book: Table of Contents</h2>

<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#about-the-project"> ➤ 프로젝트 소개</a></li>
    <li><a href="#prerequisites"> ➤ 기능 명세</a></li>
    <li><a href="#folder-structure"> ➤ 프로젝트 설계</a></li>
    <li><a href="#dataset"> ➤ 기능 설명</a></li>
    <li><a href="#roadmap"> ➤ 시연 영상</a></li>
  </ol>
</details>

![-----------------------------------------------------](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

<!-- ABOUT THE PROJECT -->
<h2 id="about-the-project"> :pencil: 프로젝트 소개</h2>

<p align="justify"> 
이 프로젝트는 GUI와 CAN 통신을 기반으로 한 실시간 출입 제어 시스템입니다. <br>
CAN 통신을 통해 출입 장치를 빠르게 제어하고, 사용자 친화적인 인터페이스를 제공하여 출입 관련 알림을 쉽게 볼 수 있게 합니다. 
  <br>PyQT를 사용한 GUI는 직관적이고, CAN 통신을 통해 안정적인 데이터 전송이 가능합니다.
</p>

### 프로토콜 별 사용 이유

#### CAN
* 3개의 라즈베리파이 통신을 효율적으로 구축하기 위해 사용
  * 우선순위 기반의 메시지 전송을 통해 중요한 데이터가 먼저 전송되도록 하여 실시간 성능을 보장
  * 물리적 결함에 강한 네트워크 구조를 가지고 있어, 일부 노드의 장애가 전체 네트워크에 영향을 주지 않음
  * BUS를 기반으로, 각 장치가 고유한 메시지 ID를 사용하여 통신하기 때문에 시스템 확장이 용이함

#### Ethernet
  - 서버에 접근하기 위해 인터넷을 활용해야 했기 때문

#### Serial
* 확실한 제어를 위해 무선 통신보다 비교적 신뢰성이 높은 유선 통신 프로토콜 사용



<!-- ROADMAP -->
<h2 id="roadmap"> :dart: 시연 영상</h2>
https://youtu.be/gD510a-3kjQ


