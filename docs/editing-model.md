# Editing model: everything is undoable, nothing is baked in until output

## The operation log is the source of truth

The image on screen while editing is not the working state — the operation
log (`ops_` in `CaptureEditor`, an ordered `QVector<Operation>` with an
`opIndex_` cursor) is. Every action that changes the picture — a crop, a
background choice, adding/patching/deleting an annotation, a cut — appends
one `Operation`. Undo moves `opIndex_` back; redo moves it forward.
`CaptureEditor::replayLog()` rebuilds the entire visible state (selection,
background, annotation list, cuts) by replaying the log from empty up to
`opIndex_`, every time. There is no separate "current annotations" model
that the log merely mirrors — the log **is** the model.

This is why undo is exact and unlimited within a session, why the crash
recovery snapshot (`snapshotWatcher_`) can save just the source image plus
the JSON-serialized log and reconstruct everything on reopen (see
`capture-handoff.hpp`), and why new annotation kinds or edit actions must be
expressed as a new `Operation::Type` (or a field on `Annotation`) rather
than a side channel of editor state — anything that bypasses the log breaks
undo, crash recovery, and notification editing simultaneously.

## Rendering is a pure function of the log, until you export

`renderCapture(capture, selection, annotations, backgroundStyle)` in
`src/capture.cpp` takes the pristine source image and the current vector
state and produces a flattened `QImage`. Output workers call it when they
need a complete raster. Interactive painting follows the same layer order
but keeps the unannotated/redacted display base cached and paints vectors
only into Qt's damaged region; pointer motion on a 6K display must not
flatten or repaint the full capture. **Nothing is baked into the working
image as you draw.** Add a rectangle, change your mind, delete it — the
source pixels underneath were never touched.

Completing a fresh capture copies it without exporting a file and shows one
click-to-edit notification. Autosave is off by default.
`--editor overlay` or `--editor window` opens annotation before output; `E` or
`A` in the capture picker toggles the same behavior for that capture, including
scrolling captures. Copy closes the editor silently. Save, Save As, and Copy +
Save write the file and show a notification. Cancelling the chooser or failing
to save keeps editing open. Rendering and output run off the UI thread (see
[threading.md](threading.md)).

Notifications use a temporary source, operation log, and thumbnail under the
private runtime directory. There is no screenshot history, and closing an editor
or using Copy does not retain another capture. Edit waits for any pending handoff
write so native pixels and layers survive.
Expired action documents are pruned after 15 minutes when new data is published;
the runtime directory is also cleared with the login session.
