internal v3
GetUnitPosition(hex_grid * Grid, unit Unit) {
    v3 Position = Grid->Cells[Unit.CellIndex].Position;
    return Position;
}
