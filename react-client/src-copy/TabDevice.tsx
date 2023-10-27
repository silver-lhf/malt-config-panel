import { useEffect, useState } from "react";

interface PinConfig {
  pin: number;
  pinEnable: boolean;
  pinReadWrite: number;
  pinTopic: string;
  pinEventTriggerEnable: boolean;
  pinEventTriggerType: number;
  pinEventTriggerValue: number;
}

export default function TabAPI() {
  const [pinConfig, setPinConfig] = useState<PinConfig[]>([]);

  function handlePinEnableChange(
    event: React.ChangeEvent<HTMLInputElement>,
    item: PinConfig
  ) {
    const newState = pinConfig.map((el) =>
      el.pin === item.pin ? { ...el, pinEnable: event.target.checked } : el
    );
    setPinConfig(newState);
  }
  function handlePinReadWriteChange(
    event: React.ChangeEvent<HTMLInputElement>,
    item: PinConfig
  ) {
    const newState = pinConfig.map((el) =>
      el.pin === item.pin
        ? { ...el, pinReadWrite: event.target.checked ? 1 : 0 }
        : el
    );
    setPinConfig(newState);
  }
  function handlePinTopicChange(
    event: React.ChangeEvent<HTMLInputElement>,
    item: PinConfig
  ) {
    const newState = pinConfig.map((el) =>
      el.pin === item.pin ? { ...el, pinTopic: event.target.value } : el
    );
    setPinConfig(newState);
  }

  function handlePinEventTriggerEnableChange(
    event: React.ChangeEvent<HTMLInputElement>,
    item: PinConfig
  ) {
    const newState = pinConfig.map((el) =>
      el.pin === item.pin
        ? { ...el, pinEventTriggerEnable: event.target.checked }
        : el
    );
    setPinConfig(newState);
  }
  function handlePinEventTriggerTypeChange(
    event: React.ChangeEvent<HTMLSelectElement>,
    item: PinConfig
  ) {
    const newState = pinConfig.map((el) =>
      el.pin === item.pin
        ? { ...el, pinEventTriggerType: parseInt(event.target.value) }
        : el
    );
    setPinConfig(newState);
  }

  function handlePinEventTriggerValueChange(
    event: React.ChangeEvent<HTMLInputElement>,
    item: PinConfig
  ) {
    const cap_max = item.pin == 32 || item.pin == 33 ? 255 : 1;
    const newValue = Math.max(
      Math.min(cap_max, parseInt(event.target.value)),
      0
    );
    const newState = pinConfig.map((el) =>
      el.pin === item.pin ? { ...el, pinEventTriggerValue: newValue } : el
    );
    setPinConfig(newState);
  }

  async function saveConfig(_: React.MouseEvent<HTMLElement>) {
    console.log(pinConfig);
    let result: string;
    try {
      const res = await fetch("/pin/save", {
        method: "POST",
        body: JSON.stringify(pinConfig),
      });

      if (res.status === 200) {
        result = `Saved >> ${await res.text()}`;
        alert("PIN Config Saved");
      } else {
        result = `${res.status}\n${res}`;
      }
    } catch (error) {
      result = `${error}`;
    }
    console.log(result);
  }

  useEffect(() => {
    // fetch files list
    fetch("/storage/?filepath=pin.json")
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
        let pinConfig: PinConfig[] = [];
        for (const config of rawJson) {
          let itemObj = {
            pin: config["pin"],
            pinTopic: config["pinTopic"],
            pinEnable: config["pinEnable"] == 1 ? true : false,
            pinReadWrite: config["pinReadWrite"],
            pinEventTriggerEnable:
              config["pinEventTriggerEnable"] == 1 ? true : false,
            pinEventTriggerType: config["pinEventTriggerType"],
            pinEventTriggerValue: config["pinEventTriggerValue"],
          };
          pinConfig.push(itemObj);
        }
        console.log(pinConfig);
        setPinConfig(pinConfig);
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
    <div className="container mx-auto">
      <button
        onClick={saveConfig}
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
      <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
        {pinConfig.map(
          (
            item,
            _ // change it into sub class
          ) => (
            <div className="bg-white p-4 rounded shadow">
              <p className="flex justify-evenly">
                <h2 className="text-xl font-bold">Pin #{item.pin}</h2>

                <label className="relative inline-flex items-center cursor-pointer">
                  <input
                    type="checkbox"
                    onChange={(event) => handlePinEnableChange(event, item)}
                    checked={item.pinEnable}
                    className="sr-only peer"
                  />
                  <div className="w-11 h-6 bg-gray-200 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-yellow-300 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[1px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-yellow-400"></div>
                  <span className="ml-3 text-md font-bold text-gray-900">
                    {item.pinEnable ? "On" : "Off"}
                  </span>
                </label>
              </p>
              {item.pinEnable ? (
                <div>
                  <div className="grid grid-cols-2 gap-2">
                    <label
                      htmlFor="pinTopic"
                      className="place-self-center text-sm font-medium text-gray-900 "
                    >
                      Topic
                    </label>
                    <input
                      type="text"
                      id="pinTopic"
                      value={item.pinTopic}
                      onChange={(event) => handlePinTopicChange(event, item)}
                      className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                      placeholder="Onenet Topic"
                    />
                  </div>
                  <div className="grid grid-cols-2 gap-2">
                    <label
                      htmlFor="pinReadWrite"
                      className="place-self-center text-sm font-medium text-gray-900 "
                    >
                      Mode
                    </label>

                    <label className="relative inline-flex items-center cursor-pointer">
                      <input
                        type="checkbox"
                        id="pinReadWrite"
                        onChange={(event) =>
                          handlePinReadWriteChange(event, item)
                        }
                        checked={item.pinReadWrite === 1 ? true : false}
                        className="sr-only peer"
                      />
                      <div className="w-11 h-6 bg-gray-200 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-yellow-300 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[1px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-yellow-400"></div>
                      <span className="ml-3 text-sm font-medium text-gray-900">
                        {item.pinReadWrite ? "Write" : "Read"}
                      </span>
                    </label>
                  </div>

                  {item.pinReadWrite == 0 ? (
                    <div>
                      <div className="grid grid-cols-2 gap-2">
                        <label
                          htmlFor="pinEventTriggerEnable"
                          className="place-self-center text-sm font-medium text-gray-900 "
                        >
                          Event Trigger
                        </label>

                        <label className="relative inline-flex items-center cursor-pointer">
                          <input
                            type="checkbox"
                            id="pinEventTriggerEnable"
                            onChange={(event) =>
                              handlePinEventTriggerEnableChange(event, item)
                            }
                            checked={item.pinEventTriggerEnable}
                            className="sr-only peer"
                          />
                          <div className="w-11 h-6 bg-gray-200 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-yellow-300 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[1px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-yellow-400"></div>
                          <span className="ml-3 text-sm font-medium text-gray-900">
                            {item.pinEventTriggerEnable ? "On" : "Off"}
                          </span>
                        </label>
                      </div>

                      {item.pinEventTriggerEnable ? (
                        <div>
                          <div className="grid grid-cols-2 gap-2">
                            <label
                              htmlFor="pinEventTriggerType"
                              className="place-self-center text-sm font-medium text-gray-900 "
                            >
                              Trigger Condition
                            </label>

                            <select
                              id="pinEventTriggerType"
                              className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                              value={item.pinEventTriggerType}
                              onChange={(event) =>
                                handlePinEventTriggerTypeChange(event, item)
                              }
                            >
                              <option value="0">
                                Greater than
                              </option>
                              <option value="1">
                                Equal to
                              </option>
                              <option value="2">
                                Less than
                              </option>
                            </select>
                          </div>

                          <div className="grid grid-cols-2 gap-2">
                            <label
                              htmlFor="pinEventTriggerValue"
                              className="place-self-center text-sm font-medium text-gray-900 "
                            >
                              Trigger Value
                            </label>
                            <input
                              type="number"
                              id="pinEventTriggerValue"
                              value={item.pinEventTriggerValue}
                              onChange={(event) =>
                                handlePinEventTriggerValueChange(event, item)
                              }
                              className="text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
                              min={0}
                              max={item.pin == 32 || item.pin == 33 ? 255 : 1}
                            />
                          </div>
                        </div>
                      ) : null}
                    </div>
                  ) : null}
                </div>
              ) : null}
            </div>
          )
        )}
      </div>
    </div>
  );
}
