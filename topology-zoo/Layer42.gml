graph [
  label "Layer42"
  node [
    id 0
    label "Seattle"
    graphics
    [
     x -122.33207
     y 47.60621
    ]
  ]
  node [
    id 1
    label "San Francisco"
    graphics
    [
     x -122.41942
     y 37.77493
    ]
  ]
  node [
    id 2
    label "Los Angeles"
    graphics
    [
     x -118.24368
     y 34.05223
    ]
  ]
  node [
    id 3
    label "Chicago"
    graphics
    [
     x -87.65005
     y 41.85003
    ]
  ]
  node [
    id 4
    label "New York City"
    graphics
    [
     x -74.00597
     y 40.71427
    ]
  ]
  node [
    id 5
    label "Washington DC"
    graphics
    [
     x -77.03637
     y 38.89511
    ]
  ]
  edge [
    source 0
    target 1
    id "e0"
  ]
  edge [
    source 1
    target 2
    id "e1"
  ]
  edge [
    source 1
    target 3
    id "e2"
  ]
  edge [
    source 1
    target 5
    id "e3"
  ]
  edge [
    source 3
    target 4
    id "e4"
  ]
  edge [
    source 3
    target 5
    id "e5"
  ]
  edge [
    source 4
    target 5
    id "e6"
  ]
]
