// Copyright 2024 Google LLC

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     https://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

const functions = [
  {
    name: 'set_timecodes',
    description: 'Set the timecodes for the video with associated text',
    parameters: {
      type: 'object',
      properties: {
        timecodes: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              time: {
                type: 'string'
              },
              text: {
                type: 'string'
              }
            },
            required: ['time', 'text']
          }
        }
      },
      required: ['timecodes']
    }
  },
  {
    name: 'set_timecodes_with_objects',
    description:
      'Set the timecodes for the video with associated text and object list',
    parameters: {
      type: 'object',
      properties: {
        timecodes: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              time: {
                type: 'string'
              },
              text: {
                type: 'string'
              },
              objects: {
                type: 'array',
                items: {
                  type: 'string'
                }
              }
            },
            required: ['time', 'text', 'objects']
          }
        }
      },
      required: ['timecodes']
    }
  },
  {
    name: 'set_timecodes_with_numeric_values',
    description:
      'Set the timecodes for the video with associated numeric values',
    parameters: {
      type: 'object',
      properties: {
        timecodes: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              time: {
                type: 'string'
              },
              value: {
                type: 'number'
              }
            },
            required: ['time', 'value']
          }
        }
      },
      required: ['timecodes']
    }
  }
]

export default fnMap =>
  functions.map(fn => ({
    ...fn,
    callback: fnMap[fn.name]
  }))
