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

      <button
        type="submit"
        className="text-white bg-blue-700 hover:bg-blue-800 focus:ring-4 focus:outline-none focus:ring-blue-300 font-medium rounded-lg text-sm w-full sm:w-auto px-5 py-2.5 text-center dark:bg-blue-600 dark:hover:bg-blue-700 dark:focus:ring-blue-800"
      >
        Save Wifi
      </button>
    </form>
  );
}
