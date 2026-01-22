import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
  ReferenceArea,
} from "recharts";
import './App.css'

const data = [
  { size: 1024, loop0: 725, nextloops: 5028 },
  { size: 2048, loop0: 590, nextloops: 4960 },
  { size: 4096, loop0: 580, nextloops: 5046 },
  { size: 5120, loop0: 575, nextloops: 5108 },
  { size: 6144, loop0: 567, nextloops: 4851 },
  { size: 7168, loop0: 447, nextloops: 68 }, // The Thrashing Cliff
  { size: 7552, loop0: 527, nextloops: 47 }, // The Thrashing Cliff
  { size: 8192, loop0: 400, nextloops: 54 }, // The Thrashing Cliff
];

// Compute 1 GiB ticks (value are in MiB, so step=1024 -> 1 GiB)
const minSize = Math.min(...data.map((d) => d.size));
const maxSize = Math.max(...data.map((d) => d.size));
const giBStep = 1024;

const xTicks: number[] = [];
for (let size = Math.ceil(minSize / giBStep) * giBStep; size <= maxSize; size += giBStep) {
  xTicks.push(size);
}

const App = () => {
  return (
    <div className="max-w-4xl mx-auto bg-white p-6 rounded-xl shadow-lg" >
      <h1 className="text-4xl text-center font-bold mb-2 text-gray-800" >
        OSTEP Ch 21: Performance Graph
      </h1>
      < p className="text-center text-gray-600 mb-6" >
        Visualization of Bandwidth Collapse during Memory Thrashing
      </p>

      < div className="h-96 w-full" >
        <ResponsiveContainer width="100%" height="100%" >
          <LineChart
            data={data}
            margin={{ top: 5, right: 30, left: 20, bottom: 20 }
            }
          >
            <CartesianGrid strokeDasharray="3 5" vertical={false} />
            <XAxis
              dataKey="size"
              type="number"
              scale={"linear"}
              domain={["dataMin", "dataMax"]}
              ticks={xTicks}
              tickFormatter={(value) => `${value / 1024} GiB`}
              label={{
                value: "Memory Allocated (MiB)",
                position: "insideBottomRight",
                offset: -10,
              }}
            />
            < YAxis
              label={{
                value: "Bandwidth (MiB/s)",
                angle: -90,
                position: "insideLeft",
              }}
            />
            < Tooltip />
            <Legend verticalAlign="top" height={24} />

            {/* Highlighting the Thrashing Zone */}
            < ReferenceArea
              x1={7168}
              x2={8192}
              strokeOpacity={0.3}
              fill="#fee2e2"
            />

            <Line
              type="monotone"
              dataKey="nextloops"
              name="Subsequent Loops (Warm)"
              stroke="#2563eb"
              strokeWidth={3}
              dot={{ r: 6 }}
              activeDot={{ r: 8 }}
            />
            < Line
              type="monotone"
              dataKey="loop0"
              name="First Loop (Cold/Paging)"
              stroke="#94a3b8"
              strokeWidth={2}
              strokeDasharray="5 5"
            />
          </LineChart>
        </ResponsiveContainer>
      </div>

      < div className="mt-8 grid grid-cols-1 md:grid-cols-2 gap-4" >
        < div className="p-4 bg-gray-50 rounded-lg border border-gray-200" >
          <h3 className="font-bold text-gray-800 mb-2" >Loop 0 vs Subsequent Loops (In-Memory):</h3>
          < p className="text-sm text-gray-600" >
            When the allocation fits in memory, the dotted line (Loop 0)
            is significantly slower than Loop 1 because of <b>Demand Paging</b>.
            The CPU has to stop and ask the Kernel for a physical page for
            every new virtual address. Once the pages are "resident,"
            Loop 1 runs at near-hardware speeds (the difference between&nbsp;
            <b>~500 MiB/s</b> and <b>~5000 MiB/s</b>).
          </p>
        </div>
        <div className="p-4 bg-blue-50 rounded-lg border border-blue-100" >
          <h3 className="font-bold text-blue-800 mb-2" >
            The "Knee" of the Curve
          </h3>
          < p className="text-sm text-blue-700" >
            When you went to <b>7GB+</b>, the bandwidth numbers didn't just drop—they
            <b> collapsed</b>. You went from <b>5000 MiB/s</b> (RAM speed)
            to <b>68-47 MiB/s</b> (Disk speed). That is a <b>100x performance
              penalty</b>.
          </p>
        </div>
      </div>
      <div className="mt-4 p-4 bg-gradient-to-r from-yellow-50 to-white rounded-lg border border-yellow-200 shadow-sm" >
        <h3 className="font-bold text-yellow-800 mb-2" >
          The Comparison
        </h3>
        <p className="text-sm text-yellow-900" >
          Performance when swapping is non-deterministic and "stifled." The
          bandwidth isn't limited by your CPU's ability to increment integers,
          but by the SSD's ability to move blocks.
        </p>
      </div>
    </div>
  );
};

export default App;
