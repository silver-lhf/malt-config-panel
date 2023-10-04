import { useEffect, useState } from "react";

interface APIHistory {
  id: number;
  name: string;
}

export default function TabAPI() {
  const [api, setAPI] = useState({
    name: "",
    method: "GET",
    url: "",
    apikey: "",
    header: [{ key: "", value: "" }],
    payload: "",
    response: "",
  });
  const [apiHistory, setAPIHistory] = useState<APIHistory[]>([]);

  function api2string() {
    return `
    {
      name: ${api.name},  
      method: ${api.method},  
      url: ${api.url},  
      apikey: ${api.apikey},  
      header: 
        {
          ${api.header[0].key}:${api.header[0].value}
        }, 
        payload: ${api.payload},  
    }
    `;
  }

  function handleNameChange(event: React.ChangeEvent<HTMLInputElement>) {
    setAPI((currentValue) => ({
      ...currentValue,
      name: event.target.value,
    }));
  }
  function handleMethodChange(event: React.ChangeEvent<HTMLSelectElement>) {
    setAPI((currentValue) => ({
      ...currentValue,
      method: event.target.value,
    }));
  }
  function handleURLChange(event: React.ChangeEvent<HTMLInputElement>) {
    setAPI((currentValue) => ({
      ...currentValue,
      url: event.target.value,
    }));
  }
  function handleAPIKeyChange(event: React.ChangeEvent<HTMLInputElement>) {
    setAPI((currentValue) => ({
      ...currentValue,
      apikey: event.target.value,
    }));
  }
  function handelAPIKeyAppend() {
    let _header = api.header;
    _header.push({ key: "", value: "" });

    setAPI((currentValue) => ({
      ...currentValue,
      header: _header,
    }));
  }
  function handleHeaderChange(
    event: React.ChangeEvent<HTMLInputElement>,
    index: number
  ) {
    let _header = api.header;
    _header[index]["key"] = event.target.value;
    setAPI((currentValue) => ({
      ...currentValue,
      header: _header,
    }));
  }
  function handleHeaderValueChange(
    event: React.ChangeEvent<HTMLInputElement>,
    index: number
  ) {
    let _header = api.header;
    _header[index]["value"] = event.target.value;
    setAPI((currentValue) => ({
      ...currentValue,
      header: _header,
    }));
  }
  function handlePayloadChange(event: React.ChangeEvent<HTMLTextAreaElement>) {
    setAPI((currentValue) => ({
      ...currentValue,
      payload: event.target.value,
    }));
  }

  async function saveRequest(_: React.MouseEvent<HTMLElement>) {
    let tosave_api = {
      name: api.name,
      method: api.method,
      url: api.url,
      apikey: api.apikey,
      header: api.header,
      payload: api.payload,
    };

    let result: string;
    console.log(`/api/save?apiName=${api.name}`);
    try {
      const res = await fetch(`/api/save?apiName=${api.name}`, {
        method: "POST",
        body: JSON.stringify(tosave_api),
      });

      if (res.status === 200) {
        result = `Successful Saved >> ${await res.text()}`;
      } else {
        result = `${res.status}\n${res}`;
      }
    } catch (error) {
      result = `${error}`;
    }

    setAPI((currentValue) => ({
      ...currentValue,
      response: result,
    }));
  }

  function runRequest(_: React.MouseEvent<HTMLElement>) {
    if (!api.name) {
      setAPI((currentValue) => ({
        ...currentValue,
        response: `Run request failed: Missing Api <Name>`,
      }));
      return;
    }

    if (!api.url) {
      setAPI((currentValue) => ({
        ...currentValue,
        response: `Run request failed: Missing Api <URL>`,
      }));
      return;
    }

    executeAPI();
  }

  async function executeAPI() {
    console.log("EXECUTE");

    let _url = api.url;
    let _method = api.method;

    let _header: { [key: string]: string } = {};
    for (const item of api.header) {
      if (item.key.trim().length == 0 || item.value.trim().length == 0) {
        continue;
      }

      _header[item.key] = item.value;
    }

    if (api.apikey) {
      _header["api-key"] = api.apikey.trim();
    }

    let _body;
    if (api.payload) {
      _header["Content-Type"] = "text/plain";
      _body = api.payload.trim();
    }

    let result: string;
    try {
      const res = await fetch(_url, {
        method: _method,
        headers: _header,
        body: _body,
      });

      if (res.status === 200) {
        const resJson = await res.json();
        result = `${JSON.stringify(resJson)}`;
      } else {
        result = `${res.status}\n${res}\n${api2string()}`;
      }
    } catch (error) {
      result = `${error}\n${api2string()}`;
    }

    setAPI((currentValue) => ({
      ...currentValue,
      response: result,
    }));
  }

  useEffect(() => {
    // fetch files list
    fetch("/api/load")
      .then((res) => {
        if (res.ok) {
          return res.text();
        }
      })
      .then((resText) => {
        console.log(resText);

        if (resText === undefined) {
          return;
        }

        let rawJson = JSON.parse(resText);
        let history: APIHistory[] = [];
        Object.keys(rawJson).map((item) => {
          const fields = item.slice(0, -5).split("_");
          let itemObj = {
            id: Number(fields[1]),
            name: fields[2],
          };
          history.push(itemObj);
        });
        setAPIHistory(history);
        console.log(history);
        console.log(apiHistory);
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
    <div>
      <div className="grid grid-row lg:grid-cols-2 gap-12">
        <div className="w-full rounded shadow-lg">
          <div className="px-6 py-4">
            {/* <form id="apiform" onSubmit={runRequest}> */}
            <div className="grid grid-cols-3 gap-2">
              <label
                htmlFor="name"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                Name
              </label>
              <input
                type="text"
                id="name"
                name="name"
                value={api.name}
                onChange={handleNameChange}
                className="col-span-2 text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                placeholder="Butterfly count"
                required
              />
            </div>
            <div className="grid grid-cols-3 gap-2">
              <label
                htmlFor="method"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                Method
              </label>
              <select
                id="method"
                value={api.method}
                onChange={handleMethodChange}
                className="bg-gray-50 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5"
              >
                <option value="GET" selected>
                  GET
                </option>
                <option value="POST">POST</option>
              </select>
            </div>
            <div className="grid grid-cols-3 gap-2">
              <label
                htmlFor="url"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                URL
              </label>
              <input
                type="text"
                id="url"
                value={api.url}
                name="url"
                onChange={handleURLChange}
                className="col-span-2 text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                placeholder="http://api.example/"
                required
              />
            </div>
            <div className="grid grid-cols-3 gap-2">
              <label
                htmlFor="apikey"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                APIKey
              </label>
              <input
                type="text"
                id="apikey"
                value={api.apikey}
                onChange={handleAPIKeyChange}
                className="col-span-2 text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                placeholder="your-api-key"
              />
            </div>
            <div className="grid grid-cols-3 gap-1">
              <label
                htmlFor="header"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                Header
                <button
                  style={buttonStyle}
                  onClick={handelAPIKeyAppend}
                  className="hover:bg-yellow-700 font-bold rounded-full"
                >
                  +
                </button>
              </label>
              {api.header.map((item, index) => {
                if (index === 0) {
                  return (
                    <div className="col-span-2 grid grid-cols-2 gap-2">
                      <input
                        type="text"
                        id={`header-${index}`}
                        value={item.key}
                        onChange={(e) => handleHeaderChange(e, index)}
                        className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                        placeholder="key"
                      />
                      <input
                        type="text"
                        id={`header-value-${index}`}
                        value={item.value}
                        onChange={(e) => handleHeaderValueChange(e, index)}
                        className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                        placeholder="value"
                      />
                    </div>
                  );
                } else {
                  return (
                    <div className="col-start-2 col-span-2 grid grid-cols-2 gap-2">
                      <input
                        type="text"
                        id={`header-${index}`}
                        value={item.key}
                        onChange={(e) => handleHeaderChange(e, index)}
                        className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                        placeholder="key"
                      />
                      <input
                        type="text"
                        id={`header-value-${index}`}
                        value={item.value}
                        onChange={(e) => handleHeaderValueChange(e, index)}
                        className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                        placeholder="value"
                      />
                    </div>
                  );
                }
              })}
            </div>
            <div className="grid grid-cols-3 gap-2">
              <label
                htmlFor="payload"
                className="place-self-center text-sm font-medium text-gray-900 "
              >
                Payload
              </label>
              <textarea
                rows={16}
                id="payload"
                value={api.payload}
                onChange={handlePayloadChange}
                className="col-span-2 resize-none h-full text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                placeholder="{ ... body }"
              />
            </div>
            <div className="mt-2 flex flex-row">
              <button
                onClick={saveRequest}
                style={buttonStyle}
                className="mx-2 font-bold py-2 px-4 rounded"
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

              <button
                style={buttonStyle}
                className="mx-2 font-bold py-2 px-4 rounded"
                // type="submit"
                // form="apiform"
                // value="Submit"
                onClick={runRequest}
              >
                Run request
              </button>
            </div>
            {/* </form> */}
          </div>
        </div>

        <div className="w-full rounded shadow-lg">
          <div className="px-6 py-4">
            <label
              htmlFor="response"
              className="mb-6 text-left block text-sm font-medium text-gray-900 "
            >
              Response
            </label>
            <textarea
              rows={28}
              id="response"
              disabled
              value={api.response}
              className="block resize-none text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
              placeholder="{ ... response }"
            />
          </div>
        </div>
      </div>
      {/* <ul className="max-w-xs text-left font-medium text-md leading-none border-yellow-400 divide-y divide-yellow-400">
        {apiRecord.map((api_name) => {
          // console.log(api_name);
          return (
            <li>
              <a
                className="py-3.5 w-full flex items-center hover:text-yellow-700 hover:bg-yellow-50"
                href="#"
              >
                <span className="ml-5 mr-2.5 w-1 h-7 bg-yellow-500 rounded-r-md"></span>
                {api_name.substring(0, api_name.length - 5)}
              </a>
            </li>
          );
        })}
      </ul> */}
      {/* <div className="flex flex-col">
        <div className="overflow-x-auto sm:-mx-6 lg:-mx-8">
          <div className="inline-block min-w-full py-2 sm:px-6 lg:px-8">
            <div className="overflow-hidden">
              <table className="min-w-full text-left text-sm font-light">
                <thead className="border-b font-medium dark:border-neutral-500">
                  <tr>
                    <th scope="col" className="px-6 py-4">
                      #
                    </th>
                    <th scope="col" className="px-6 py-4">
                      First
                    </th>
                    <th scope="col" className="px-6 py-4">
                      Last
                    </th>
                    <th scope="col" className="px-6 py-4">
                      Handle
                    </th>
                  </tr>
                </thead>
                <tbody>
                  <tr className="border-b transition duration-300 ease-in-out hover:bg-neutral-100 dark:border-neutral-500 dark:hover:bg-neutral-600">
                    <td className="whitespace-nowrap px-6 py-4 font-medium">
                      1
                    </td>
                    <td className="whitespace-nowrap px-6 py-4">Mark</td>
                    <td className="whitespace-nowrap px-6 py-4">Otto</td>
                    <td className="whitespace-nowrap px-6 py-4">@mdo</td>
                  </tr>
                  <tr className="border-b transition duration-300 ease-in-out hover:bg-neutral-100 dark:border-neutral-500 dark:hover:bg-neutral-600">
                    <td className="whitespace-nowrap px-6 py-4 font-medium">
                      2
                    </td>
                    <td className="whitespace-nowrap px-6 py-4">Jacob</td>
                    <td className="whitespace-nowrap px-6 py-4">Thornton</td>
                    <td className="whitespace-nowrap px-6 py-4">@fat</td>
                  </tr>
                  <tr className="border-b transition duration-300 ease-in-out hover:bg-neutral-100 dark:border-neutral-500 dark:hover:bg-neutral-600">
                    <td className="whitespace-nowrap px-6 py-4 font-medium">
                      3
                    </td>
                    <td className="whitespace-nowrap px-6 py-4">Larry</td>
                    <td className="whitespace-nowrap px-6 py-4">Wild</td>
                    <td className="whitespace-nowrap px-6 py-4">@twitter</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div> */}
    </div>
  );
}
