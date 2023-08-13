import { useState } from "react";

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

  async function saveRequest(event: React.MouseEvent<HTMLElement>) {
    console.log("SAVE", typeof event, event);

    let result: string;
    try {
      const res = await fetch("/api/save", {
        method: "POST",
        body: JSON.stringify(api),
      });

      if (res.status === 200) {
        result = `Successful Saved`;
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

  function runRequest(event: React.MouseEvent<HTMLElement>) {
    console.log("RUN", typeof event, event);

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

  async function executeAPI(): Promise<void> {
    console.log("EXECUTE");

    let _url = api.url;
    let _method = api.method;

    let _header: { [key: string]: string } = {};
    for (const item of api.header) {
      if (item.key.trim().length == 0 || item.value.trim().length == 0) {
        continue;
      }

      console.log(item.key, item.value);
      _header[item.key] = item.value;
    }

    // if (api.apikey) {
    //   console.log("APIkey", api.apikey);
    // _header["X-Auth-Token"] = api.apikey;
    // }

    // if (api.payload) {
    //   console.log("payload", api.payload);
    //   _body = JSON.stringify(api.payload);
    // }

    console.log(_url, _method);

    let result: string;
    try {
      const res = await fetch(_url, {
        method: _method,
        // headers: _header,
        // body: _body,
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
  const buttonStyle = {
    color: "#FFAC0A",
    background: "#FFFBEC",
  };

  return (
    <div className="grid grid-cols-2 gap-12">
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
              style={buttonStyle}
              className="mx-2 font-bold py-2 px-4 rounded"
              onClick={saveRequest}
            >
              Save
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
  );
}
