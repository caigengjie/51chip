#include <ESP8266WiFi.h> 
#include <ESPAsyncWebServer.h>    // 包含异步Web服务器库文件

char ssid[] = "*";         // WiFi名
char pass[] = "*";         // WiFi密码
int Humidity = 0;
int Temperature = 0;
int RH=0,RL=0,TH=0,TL=0;

AsyncWebServer server(80);        // 创建WebServer对象, 端口号80
// 使用端口号80可以直接输入IP访问，使用其它端口需要输入IP:端口号访问
// 一个储存网页的数组
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
	<meta charset="utf-8">
    <title>登录</title>
    <style>
        *{
            text-align: center;
        }
        body{
            background: linear-gradient(to right, #eeeeee, #c2c2c2);
        }
        #login{
            margin: 150px auto;
            padding: 20px;
            height: 300px;
            width: 400px;
            background-color: #ffffff;
            border-radius: 25px;
			box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
        }
        #pass,#uname{
            margin: 25px auto;
            display: block;
            line-height: 1.3;
            height: 35px;
            width: 300px;
            border-width: 1px;
            border-style: solid;
            
        }
        .btn {
            margin: 25px auto;
            display: block;
            height: 38px;
            line-height: 38px;
            padding: 0 18px;
            border: 1px solid transparent;
            background-color: #4476A7;
            color: #fff;
            text-align: center;
            font-size: 14px;
            border-radius: 2px;
        }
        p{
            margin: 20px auto;
            font-size: 30px;
        }
    </style>
</head>
<body>
	<div id="login">
        <p>数据网页</p>
        <div>
            <input type="text" name="username" id="uname" placeholder="用户名">
        </div>
        <div>
            <input type="password" name="password" id="pass" placeholder="密码">
        </div>
        <div>
            <button class="btn" onclick="set()" id="button"> 登 录 </button>
        </div>
    </div>
</body>
<script>
	// 按下按钮会运行这个JS函数
	function set() {
		var p1 = document.getElementById("uname").value; // 需要发送的内容
    var p2 = document.getElementById("pass").value;
		// 通过get请求给 /set
		var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
			if (this.readyState == 4 && this.status == 200) {
				// 跳转网页
				window.location.href="http://192.168.255.176/index";
			}
      else if (this.readyState == 4 && this.status == 404)
        alert("用户名或密码错误！");
		};
		xhr.open("GET", "/set?value=" + p1 + p2, true);
		xhr.send();
	}
</script>)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
	<meta charset="utf-8">
    <title>传感器数据网页</title>
    <style>
		body{
			background-color: #ededed;
		}
        h2{
			margin: 30px auto;
            text-align: center;
        }
		p{
			margin: 0;
		}
		#sensor{
			height: 350px;
			width: 600px;
			margin: 50px auto;
		}
		#sensor-temperature{
			height: 300px;
			width: 200px;	
      transition: width 1.5s, height 1.5s;
			float: left;
			border-radius: 25px;
			box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
		}
		#temp01{
			height: 20%;
			text-align: center;
			background-color: rgb(0, 211, 211);
			border-radius:25px 25px 0 0;
			line-height: 55px;
			font-size: 24px;
			color: #ffffff;
		}
		#temp02{
			height: 80%;
			border-radius:0 0 25px 25px;
			background-color: rgb(0, 243, 243);
			text-align: center;
			line-height: 55px;
			font-size: 25px;
			color: #ffffff;
		}
		#sensor-humidity{
			height: 300px;
			width: 200px;	
      transition: width 1.5s, height 1.5s;
			float: right;
			border-radius: 25px;
			box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
		}
		#humi01{
			height: 20%;
			text-align: center;
			background-color: rgb(0, 211, 211);
			border-radius:25px 25px 0 0;
			line-height: 55px;
			font-size: 24px;
			color: #ffffff;
		}
		#humi02{
			height: 80%;
			border-radius:0 0 25px 25px;
			background-color: rgb(0, 243, 243);
			text-align: center;
			line-height: 55px;
			font-size: 25px;
			color: #ffffff;
		}
    #temp, #humi{
			font-size: 60px;
		}
		#sensor-temperature:hover, #sensor-humidity:hover{
			height: 325px;
			width: 225px;
		}
    </style>
</head>
<body>
	<h2>传感器数据网页</h2>
    <hr>
	<div id="sensor">
		<div id="sensor-temperature">
			<div id="temp01">
				温度
			</div>
			<div id="temp02">
				<p>当前温度为：</p>
				<br>
				<p id="temp">20</p>
			</div>
		</div>
		<div id="sensor-humidity">
			<div id="humi01">
				湿度
			</div>
			<div id="humi02">
				<p>当前湿度为：</p>
				<br>
				<p id="humi">20</p>
			</div>
		</div>
	</div>
</body>
<script>
	// 设置一个定时任务, 1000ms执行一次
	setInterval(function () {
		var xhttp = new XMLHttpRequest();
		xhttp.onreadystatechange = function () {
			if (this.readyState == 4 && this.status == 200) {
				// 此代码会搜索ID为temp的组件，然后使用返回内容替换组件内容
				document.getElementById("temp").innerHTML = this.responseText;
			}
		};
		// 使用GET的方式请求 /temp
		xhttp.open("GET", "/temp", true);
		xhttp.send();
	}, 1000)
	setInterval(function () {
		var xhttp = new XMLHttpRequest();
		xhttp.onreadystatechange = function () {
			if (this.readyState == 4 && this.status == 200) {
				// 此代码会搜索ID为humi的组件，然后使用返回内容替换组件内容
				document.getElementById("humi").innerHTML = this.responseText;
			}
		};
		// 使用GET的方式请求 /humi
		xhttp.open("GET", "/humi", true);
		xhttp.send();
	}, 1000)
</script>)rawliteral";

void Config_Callback(AsyncWebServerRequest *request)
{
  if (request->hasParam("value")) // 如果有值下发
  {
    String HTTP_Payload = request->getParam("value")->value();    // 获取下发的数据
    if(HTTP_Payload=="admin1234")
      request->send(200, "text/plain", "OK"); 
    else
      request->send(404, "text/plain", "OK");
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);      //  等待端口的释放
  WiFi.begin(ssid, pass);         // 连接WiFi
  while (WiFi.status() != WL_CONNECTED) {       // 如果WiFi没有连接，一直循环打印点
    delay(500);
    Serial.print(".\n");
  } 
  Serial.println(WiFi.localIP());     // 打印开发板的IP地址
 
  // 添加HTTP主页，当访问的时候会把网页推送给访问者
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", login_html); });
  server.on("/set", HTTP_GET, Config_Callback);   // 绑定配置下发的处理函数  
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });
  // 设置反馈的信息，在HTML请求链接时，返回打包好的传感器数据
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(Temperature).c_str()); });
  server.on("/humi", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(Humidity).c_str()); });

  server.begin();  // 初始化HTTP服务器
}
void loop()
{
  if (Serial.available()>4)
  {
    RH = Serial.read()-'0';
    RL = Serial.read()-'0';
    TH = Serial.read()-'0';
    TL = Serial.read()-'0';
    Humidity = RH*10+RL;
    Temperature = TH*10+TL;
  }  
}
