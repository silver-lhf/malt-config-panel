import { useEffect, useState } from "react";

export default function TabConfig() {
  const [productID, setProductID] = useState("");
  const [deviceID, setDeviceID] = useState("");
  const [password, setPassword] = useState("");
  const [transferInterval, setTransferInterval] = useState(10000);

  const buttonStyle = {
    color: "#FFAC0A",
    background: "#FFFBEC",
  };

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    const params = new URLSearchParams({
      productID: productID,
      deviceID: deviceID,
      password: password,
      interval: String(transferInterval),
    });

    fetch(`/config?${params}`)
      .then((res) => {
        if (!res.ok) {
          throw new Error("Error setup config");
        }else{
          alert("Config Saved!");
        }
      })
      .catch((e) => {
        console.log(e);
      });
  };

  useEffect(() => {
    fetch("/config")
      .then((res) => {
        if (res.ok) {
          return res.json();
        }
      })
      .then((resJson) => {
        setProductID(resJson["productID"]);
        setDeviceID(resJson["deviceID"]);
        setPassword(resJson["password"]);
        setTransferInterval(parseInt(resJson["interval"]));
      })
      .catch((e) => {
        console.log(e);
      });
  }, []);

  return (
    <form onSubmit={handleSubmit}>
      <div className="mb-6">
        <label
          htmlFor="productID"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Onenet Product ID
        </label>
        <input
          type="text"
          id="productID"
          name="productID"
          value={productID}
          onChange={(e) => setProductID(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="16001234"
        />
      </div>
      <div className="mb-6">
        <label
          htmlFor="deviceID"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Device ID
        </label>
        <input
          type="text"
          id="deviceID"
          name="deviceID"
          value={deviceID}
          onChange={(e) => setDeviceID(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="161123456"
        />
      </div>
      <div className="mb-6">
        <label
          htmlFor="password"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Device Password
        </label>
        <input
          type="password"
          id="password"
          name="password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="********"
        />
      </div>

      <div className="mb-6">
        <label
          htmlFor="interval"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Transfer Interval (ms)
        </label>
        <input
          type="number"
          id="interval"
          name="interval"
          value={transferInterval}
          onChange={(e) => setTransferInterval(Number(e.target.value))}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="10000"
        />
      </div>

      <button
        type="submit"
        style={buttonStyle}
        className="mx-auto hover:bg-yellow-300 font-bold py-2 px-4 rounded items-center flex w-auto"
      >
        Save
      </button>
    </form>
  );
}
