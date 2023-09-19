import { useState, useEffect } from "react";

// function FolderBar(fname: string): JSX.Element {
//   if (fname.includes(".")) {
//     return <span></span>;
//   } else {
//     return (
//       <span className="ml-5 mr-2.5 w-1 h-7 bg-yellow-500 rounded-r-md"></span>
//     );
//   }
// }

export default function TabStorage() {
  // const [files, setFiles] = useState<string[]>([]);
  const [files, setFiles] = useState({});
  const [fileContent, setFileContent] = useState(String);
  const colorStyle = {
    color: "#FFAC0A",
  };
  // divide-y divide-yellow-400
  function fileTree(fileJson: any, path: String) {
    return (
      <ul className="max-w-xs text-left font-medium text-md leading-none border-yellow-400">
        {Object.keys(fileJson).map((item) => {
          if (item.includes(".")) {
            // File
            return (
              <li>
                <a
                  style={colorStyle}
                  className="py-3.5 w-full flex items-center hover:text-yellow-700 hover:bg-yellow-50"
                  href="#"
                  onClick={(e) => {
                    e.preventDefault();
                    readFile(`${path}/${item}`);
                  }}
                >
                  {item}
                </a>
              </li>
            );
          } else {
            // Folder
            return (
              <span>
                <li>
                  <a
                    style={colorStyle}
                    className="py-3.5 w-full flex items-center hover:text-yellow-700 hover:bg-yellow-50"
                  >
                    {item}/
                  </a>
                </li>
                <li className="ml-6">
                  {fileTree(fileJson[item], `${path}/${item}`)}
                </li>
              </span>
            );
          }
        })}
      </ul>
    );
  }

  async function readFile(filepath: String) {
    console.log(filepath);

    const url = `/storage?filepath=${filepath}`;
    const res = await fetch(url);
    if (res.ok) {
      setFileContent(await res.text());
    } else {
      console.log(res);
      setFileContent(`Failed to load File <${filepath}>`);
    }
  }

  useEffect(() => {
    // fetch files list
    fetch("/storage")
      .then((res) => {
        if (res.ok) {
          return res.text();
        }
      })
      .then((resText) => {
        if (resText === undefined) {
          return;
        }

        let _files = JSON.parse(resText);
        setFiles(_files);
      })
      .catch((e) => {
        console.log(e);
      });
  }, []);

  return (
    <div className="grid grid-cols-2 gap-12">
      <div className="w-full rounded shadow-lg">
        <div className="px-6 py-4 relative">
          {/* <ul className="max-w-xs text-left font-medium text-md leading-none border-yellow-400 divide-y divide-yellow-400">
            {Object.keys(files).map((filename) => {
              return (
                <li>
                  <a
                    style={colorStyle}
                    className="py-3.5 w-full flex items-center hover:text-yellow-700 hover:bg-yellow-50"
                    href="#"
                  >
                     <FolderBar fname={filename} />

                    <span className="ml-5 mr-2.5 w-1 h-7 bg-yellow-500 rounded-r-md"></span>
                    {filename}
                  </a>
                </li>
              );
            })} 
          </ul>*/}
          {fileTree(files, "")}
        </div>
      </div>

      <div className="w-full rounded shadow-lg">
        <div className="px-6 py-4">
          <label
            htmlFor="file_content"
            className="mb-6 text-left block text-sm font-medium text-gray-900"
          >
            File Content
          </label>
          <textarea
            rows={28}
            id="file_content"
            value={fileContent}
            className="block resize-none text-left bg-gray-200 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 w-full p-2.5"
            placeholder="{ ... file_content }"
          />
        </div>
      </div>
    </div>
  );
}
