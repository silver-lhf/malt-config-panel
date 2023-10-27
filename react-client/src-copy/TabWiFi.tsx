import { useEffect, useState } from "react";

export default function TabWiFi() {
  const [ssid, setSSID] = useState("");
  const [password, setPassword] = useState("");

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    console.log("trigger");
    e.preventDefault();

    const params = new URLSearchParams({
      ssid: ssid,
      password: password,
    });
    console.log(`/wifi?${params}`);
    fetch(`/wifi?${params}`)
      .then((res) => {
        if (!res.ok) {
          throw new Error("Error setup wifi");
        } else {
          alert("WiFi Saved!");
        }
      })
      .catch((e) => {
        console.log(e);
      });
  };

  useEffect(() => {
    fetch("/wifi")
      .then((res) => {
        if (res.ok) {
          return res.json();
        }
      })
      .then((resJson) => {
        console.log(resJson);
        setSSID(resJson["ssid"]);
        setPassword(resJson["password"]);
      })
      .catch((e) => {
        console.log(e);
      });
  }, []);

  const buttonStyle = {
    color: "#FFAC0A",
    background: "#FFFBEC",
  };

  return (
    <form onSubmit={handleSubmit}>
      <div className="mb-6">
        <label
          htmlFor="ssid"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Connect SSID
        </label>
        <input
          type="text"
          id="ssid"
          name="ssid"
          value={ssid}
          onChange={(e) => setSSID(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5 dark:bg-gray-700 "
          placeholder="your-wifi-ssid"
        />
      </div>
      <div className="mb-6">
        <label
          htmlFor="password"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Password
        </label>
        <input
          type="password"
          id="password"
          name="password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5 dark:bg-gray-700 "
          placeholder="******"
        />
      </div>
      {/* <button
        type="submit"
        style={buttonStyle}
        className="font-medium rounded-lg text-sm w-full sm:w-auto px-5 py-2.5 text-center "
      >
        <svg
          xmlns="http://www.w3.org/2000/svg"
          height="1em"
          viewBox="0 0 448 512"
        >
          <path d="M433.941 129.941l-83.882-83.882A48 48 0 0 0 316.118 32H48C21.49 32 0 53.49 0 80v352c0 26.51 21.49 48 48 48h352c26.51 0 48-21.49 48-48V163.882a48 48 0 0 0-14.059-33.941zM224 416c-35.346 0-64-28.654-64-64 0-35.346 28.654-64 64-64s64 28.654 64 64c0 35.346-28.654 64-64 64zm96-304.52V212c0 6.627-5.373 12-12 12H76c-6.627 0-12-5.373-12-12V108c0-6.627 5.373-12 12-12h228.52c3.183 0 6.235 1.264 8.485 3.515l3.48 3.48A11.996 11.996 0 0 1 320 111.48z" />
        </svg>
        <span className="ml-2">Save</span>
      </button> */}

      <button
        type="submit"
        style={buttonStyle}
        className="mx-auto hover:bg-yellow-300 font-bold py-2 px-4 rounded items-center flex w-auto"
      >
        <svg
          xmlns="http://www.w3.org/2000/svg"
          height="1em"
          viewBox="0 0 448 512"
        >
          <path d="M433.941 129.941l-83.882-83.882A48 48 0 0 0 316.118 32H48C21.49 32 0 53.49 0 80v352c0 26.51 21.49 48 48 48h352c26.51 0 48-21.49 48-48V163.882a48 48 0 0 0-14.059-33.941zM224 416c-35.346 0-64-28.654-64-64 0-35.346 28.654-64 64-64s64 28.654 64 64c0 35.346-28.654 64-64 64zm96-304.52V212c0 6.627-5.373 12-12 12H76c-6.627 0-12-5.373-12-12V108c0-6.627 5.373-12 12-12h228.52c3.183 0 6.235 1.264 8.485 3.515l3.48 3.48A11.996 11.996 0 0 1 320 111.48z" />
        </svg>
        <span className="ml-2">Save</span>
      </button>
    </form>
  );
}
