import { useEffect, useState } from "react";

export default function TabConfig() {
  const [onenetProductID, setOnenetProductID] = useState("");
  const [onenetDeviceID, setOnenetDeviceID] = useState("");
  const [onenetPassword, setOnenetPassword] = useState("");
  const [onenetTransferInterval, setOnenetTransferInterval] = useState(10000);
  const [maltPinReadingInterval, setMaltPinReadingInterval] = useState(5000);
  const [maltPinRefreshingInterval, setMaltPinRefreshingInterval] =
    useState(10000);

  const buttonStyle = {
    color: "#FFAC0A",
    background: "#FFFBEC",
  };

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    const params = new URLSearchParams({
      onenetProductID: onenetProductID,
      onenetDeviceID: onenetDeviceID,
      onenetPassword: onenetPassword,
      onenetTransferInterval: String(onenetTransferInterval),
      maltPinReadingInterval: String(maltPinReadingInterval),
      maltPinRefreshingInterval: String(maltPinRefreshingInterval),
    });

    fetch(`/config?${params}`)
      .then((res) => {
        if (!res.ok) {
          throw new Error("Error setup config");
        } else {
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
        setOnenetProductID(resJson["onenetProductID"]);
        setOnenetDeviceID(resJson["onenetDeviceID"]);
        setOnenetPassword(resJson["onenetPassword"]);
        setOnenetTransferInterval(parseInt(resJson["onenetTransferInterval"]));
        setMaltPinReadingInterval(parseInt(resJson["maltPinReadingInterval"]));
        setMaltPinRefreshingInterval(
          parseInt(resJson["maltPinRefreshingInterval"])
        );
      })
      .catch((e) => {
        console.log(e);
      });
  }, []);

  return (
    <form onSubmit={handleSubmit}>
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
      <div className="my-4">
        <label
          htmlFor="pinReadingInterval"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          MALT Board - Pin Reading Interval (ms)
        </label>
        <input
          type="number"
          id="pinReadingInterval"
          name="pinReadingInterval"
          value={maltPinReadingInterval}
          onChange={(e) => setMaltPinReadingInterval(Number(e.target.value))}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="10000(ms)"
          min={5000}
          max={7200000}
        />
      </div>
      <div className="my-2">
        <label
          htmlFor="pinReadingInterval"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          MALT Page - Pin Refreshing Interval (ms)
        </label>
        <input
          type="number"
          id="maltPinRefreshingInterval"
          name="maltPinRefreshingInterval"
          value={maltPinRefreshingInterval}
          onChange={(e) => setMaltPinRefreshingInterval(Number(e.target.value))}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="10000(ms)"
          min={10000}
          max={7200000}
        />
      </div>

      <div className="my-4">
        <label
          htmlFor="onenetProductID"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Onenet - Product ID
        </label>
        <input
          type="text"
          id="onenetProductID"
          name="onenetProductID"
          value={onenetProductID}
          onChange={(e) => setOnenetProductID(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="87654321"
        />
      </div>
      <div>
        <label
          htmlFor="onenetDeviceID"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Onenet - Device ID
        </label>
        <input
          type="text"
          id="onenetDeviceID"
          name="onenetDeviceID"
          value={onenetDeviceID}
          onChange={(e) => setOnenetDeviceID(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="987654321"
        />
      </div>
      <div className="mb-6">
        <label
          htmlFor="onenetPassword"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Onenet - Password
        </label>
        <input
          type="onenetPassword"
          id="onenetPassword"
          name="onenetPassword"
          value={onenetPassword}
          onChange={(e) => setOnenetPassword(e.target.value)}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="********"
        />
      </div>

      <div className="mb-6">
        <label
          htmlFor="interval"
          className="block mb-2 text-lg float-left font-bold text-neutral-800"
        >
          Onenet - Transfer Interval (ms)
        </label>
        <input
          type="number"
          id="interval"
          name="interval"
          value={onenetTransferInterval}
          onChange={(e) => setOnenetTransferInterval(Number(e.target.value))}
          className="bg-gray-50 border border-gray-300 text-gray-400 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
          placeholder="10000"
          min={5000}
          max={7200000}
        />
      </div>

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
