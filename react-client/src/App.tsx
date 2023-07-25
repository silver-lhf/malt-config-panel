import "./App.css";
import Button from "@mui/material/Button";

function App() {
  return (
    <div className="centered">
      <div className="wrapper">
        <h1>WiFiManager</h1>
        {/* <h3>ESP32_A81B5AE0</h3> */}
        <a href="http://192.168.4.1/wifi">
          <Button>Configure WiFi</Button>
        </a>
        <br />
        <a href="http://192.168.4.1/info">
          <Button>Info</Button>
        </a>
        <br />
        <a href="http://192.168.4.1/exit">
          <Button>Exit</Button>
        </a>
        <br />
        <hr />
        <br />
        <a href="http://192.168.4.1/update">
          <Button>Update</Button>
        </a>
        <br />
      </div>
    </div>
  );
}

export default App;
