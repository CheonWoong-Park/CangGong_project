import csv
import requests
from datetime import datetime, timedelta
import json
from flask import Flask

app = Flask(__name__)

@app.route('/')
def get_weather_data():

    # api를 보낼때 현재 날짜와 시간을 알아야한다.
    current_time = datetime.now() #모듈을 이용하여 현재시간 불러오기

    #우리가 사용하는 API는 40분이 넘어야 정시 데이터가 불러지기에 저장할때, 40분전과 후로 나누어 40분 전이면 -1한 시간 후면, 그시간 그대로 저장하도록 코드를 짰다.
    if current_time.minute <= 40:
        adjusted_time = current_time - timedelta(hours=1)
        time = adjusted_time.strftime("%H") + "00"
    else:
        time = current_time.strftime("%H") + "00"

    #현재 날짜 저장하기
    date = current_time.strftime("%Y%m%d")

    #지역 데이터 리스트
    values_list = []

    #지역별로 좌표값을 CSV(엑셀과 비슷한데 쉼표로 데이터를 나눈 데이터형식)로 저장해둔후 불러온다.
    f = open('region_data.CSV', 'r')
    rdr = csv.reader(f)
    next(rdr)

    i = 1

    for line in rdr:
        #csv파일에서 region_name, x, y 값을 불러와 한행씩 불러온다.
        region_name, x, y = line
        print(f"지역별 날씨 데이터 추출 중 {i:-2}/30 : {region_name:7}")
        i += 1  
        x, y = int(x), int(y)
        
        #불러온 데이터를 통해서 API쿼리문을 작성한다. 필요한 데이터를 파라미터로 넣어주었다.
        url = "https://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getUltraSrtNcst?serviceKey=keyIsBlank&pageNo=1&numOfRows=1000&dataType=JSON"
        url += "&base_date=" + date #날짜 파라미터
        url += "&base_time=" + time #시간 파라미터
        url += "&nx=" + str(x) #x좌표
        url += "&ny=" + str(y) #y좌표

        #get메소드로 api를 호출한다.
        response = requests.get(url)
        set_values = []

        #json으로 파싱한뒤 지역 데이터 리스트에 담는다.
        json_response = json.loads(response.text)
        data_points = json_response['response']['body']['items']['item']
        for data_point in data_points:
            if data_point['category'] in ['T1H', 'RN1', 'REH', 'WSD']:
                set_values.append(float(data_point['obsrValue']))
        values_list.append(set_values)

    f.close()

    #메모리 관리를 위해 json덤프로 압축하여 리턴한다.
    return json.dumps({'weather_data': values_list}, separators=(',', ':'))

if __name__ == '__main__':
    app.run(host = "0.0.0.0", port=5000, debug=True)