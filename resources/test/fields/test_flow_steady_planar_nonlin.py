import agros2d
from time import time
start_time = time()

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "gmsh_quad_delaunay"


# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
# flow
flow = agros2d.field("flow")
flow.analysis_type = "steadystate"
flow.number_of_refinements = 2
flow.polynomial_order = 2

flow.solver = "newton"
flow.solver_parameters['tolerance'] = 0.0001
flow.solver_parameters['steps'] = 10
flow.solver_parameters['damping'] = 'automatic'
flow.solver_parameters['damping_factor'] = 1.0
flow.solver_parameters['jacobian_reuse'] = False

flow.add_boundary("Inlet", "flow_velocity", {"flow_velocity_x" : { "expression" : "cos((y-0.135)/0.035*pi/2)" }, "flow_velocity_y" : 0})
flow.add_boundary("Wall", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : 0})
flow.add_boundary("Outlet", "flow_outlet", {})

flow.add_material("Water", {"flow_density" : 1, "flow_force_x" : 0, "flow_force_y" : 0, "flow_viscosity" : 0.001})

# geometry
geometry = agros2d.geometry
geometry.add_edge(-0.35, 0.05, 0.4, 0.05, boundaries = {"flow" : "Wall"})
geometry.add_edge(0.4, 0.05, 0.4, 0.25, boundaries = {"flow" : "Wall"})
geometry.add_edge(-0.25, 0.1, -0.2, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(-0.2, 0.1, -0.2, 0.2, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(-0.2, 0.2, -0.25, 0.2, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(-0.25, 0.2, -0.25, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(-0.35, 0.2, -0.05, 0.3, boundaries = {"flow" : "Wall"})
geometry.add_edge(-0.05, 0.3, 0, 0.15, boundaries = {"flow" : "Wall"})
geometry.add_edge(0, 0.15, 0.05, 0.1, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(0.05, 0.1, 0.1, 0.15, boundaries = {"flow" : "Wall"}, angle = 90)
geometry.add_edge(0.2, 0.3, 0.1, 0.15, boundaries = {"flow" : "Wall"})
geometry.add_edge(0.3, 0.3, 0.3, 0.25, boundaries = {"flow" : "Wall"})
geometry.add_edge(0.3, 0.25, 0.4, 0.25, boundaries = {"flow" : "Wall"})
geometry.add_edge(0.2, 0.3, 0.3, 0.3, boundaries = {"flow" : "Outlet"})
geometry.add_edge(-0.4, 0.17, -0.4, 0.1, boundaries = {"flow" : "Inlet"})
geometry.add_edge(-0.35, 0.2, -0.35, 0.17, boundaries = {"flow" : "Wall"})
geometry.add_edge(-0.4, 0.17, -0.35, 0.17, boundaries = {"flow" : "Wall"})
geometry.add_edge(-0.4, 0.1, -0.35, 0.1, boundaries = {"flow" : "Wall"})
geometry.add_edge(-0.35, 0.1, -0.35, 0.05, boundaries = {"flow" : "Wall"})

geometry.add_label(-0.086153, 0.205999, materials = {"flow" : "Water"}, area = 0.00048)
geometry.add_label(-0.224921, 0.126655, materials = {"flow" : "none"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = flow.local_values(-0.023486, 0.117842)
testp = agros2d.test("Pressure", point["p"], 0.65205)
testv = agros2d.test("Velocity", point["v"], 0.406233)
testvx = agros2d.test("Velocity - x", point["vx"], 0.337962)
testvy = agros2d.test("Velocity - y", point["vy"], -0.225404)

# volume integral
volume = flow.volume_integrals([0])
# testPj = agros2d.test("Losses", volume["Pj"], 10070.23937)

# surface integral
surface = flow.surface_integrals([0])
# testI = agros2d.test("Current", surface["Ir"], 3629.425713)

elapsed_time = time() - start_time
print("Test: Incompressible flow - plana ({0}): ".format(round(elapsed_time, 3)) + str(testp and testv and testvx and testvy))