import { useRef } from "react";

export default function TabTemplate() {
  const fileInputRef = useRef<HTMLInputElement>(null);

  const handleFileInputChange = (
    event: React.ChangeEvent<HTMLInputElement>
  ) => {
    const _file = event.target.files?.[0] || null;
    if (!_file) {
      return;
    }

    console.log("Selected file:", _file);
    const data = new FormData();
    data.append(_file.name, _file);
    fetch("/import", { method: "POST", body: data })
      .then((res) => {
        if (res.ok) {
          alert("File uploaded");
          window.location.reload();
        } else {
          throw new Error("Error uploading file");
        }
      })
      .catch((error) => {
        console.error("Error uploading file:", error);
      });
  };

  const handleExportTemplate = () => {
    fetch("/export")
      .then((res) => {
        if (res.ok) {
          return res.blob();
        } else {
          throw new Error("Error downloading file");
        }
      })
      .then((blob) => {
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "export.txt";

        a.click();
        window.URL.revokeObjectURL(url);
      })
      .catch((error) => {
        console.error("Error downloading file:", error);
      });
  };

  return (
    <div>
      <div className="grid grid-flow-row auto-rows-max">
        <div
          className="w-full rounded shadow-lg py-8"
          onClick={(_) => {
            fileInputRef.current?.click();
          }}
        >
          <input
            style={{ display: "none" }}
            type="file"
            ref={fileInputRef}
            onChange={handleFileInputChange}
          ></input>

          <div className="place-self-center text-md font-medium text-gray-900 my-3">
            Import Template
          </div>
          <span className="inline-flex justify-center items-center my-6">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              height="8em"
              viewBox="0 0 512 512"
            >
              <path d="M128 64c0-35.3 28.7-64 64-64H352V128c0 17.7 14.3 32 32 32H512V448c0 35.3-28.7 64-64 64H192c-35.3 0-64-28.7-64-64V336H302.1l-39 39c-9.4 9.4-9.4 24.6 0 33.9s24.6 9.4 33.9 0l80-80c9.4-9.4 9.4-24.6 0-33.9l-80-80c-9.4-9.4-24.6-9.4-33.9 0s-9.4 24.6 0 33.9l39 39H128V64zm0 224v48H24c-13.3 0-24-10.7-24-24s10.7-24 24-24H128zM512 128H384V0L512 128z" />
            </svg>
          </span>
        </div>
        <div
          className="w-full rounded shadow-lg py-8"
          onClick={handleExportTemplate}
        >
          <div className="place-self-center text-md font-medium text-gray-900 my-3">
            Export Template
          </div>
          <span className="inline-flex justify-center items-center my-6">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              height="8em"
              viewBox="0 0 576 512"
            >
              <path d="M0 64C0 28.7 28.7 0 64 0H224V128c0 17.7 14.3 32 32 32H384V288H216c-13.3 0-24 10.7-24 24s10.7 24 24 24H384V448c0 35.3-28.7 64-64 64H64c-35.3 0-64-28.7-64-64V64zM384 336V288H494.1l-39-39c-9.4-9.4-9.4-24.6 0-33.9s24.6-9.4 33.9 0l80 80c9.4 9.4 9.4 24.6 0 33.9l-80 80c-9.4 9.4-24.6 9.4-33.9 0s-9.4-24.6 0-33.9l39-39H384zm0-208H256V0L384 128z" />
            </svg>
          </span>
        </div>
      </div>
    </div>
  );
}
