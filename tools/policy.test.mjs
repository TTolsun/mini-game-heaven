import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {accept, check, snapshot, normalizedFacts} from './docs-review.mjs';
import {validateRelease} from './release.mjs';

function fixture(t) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'sdd-policy-test-'));
  t.after(() => {
    const absolute = path.resolve(dir);
    assert.equal(path.dirname(absolute), path.resolve(os.tmpdir()));
    assert.ok(path.basename(absolute).startsWith('sdd-policy-test-'));
    fs.rmSync(absolute, {recursive: true, force: true});
  });
  execFileSync('git', ['init', '-q', dir]);
  for (const name of ['native', 'docs/sdd', 'app/src/main', 'tools']) fs.mkdirSync(path.join(dir, name), {recursive: true});
  const write = (name, data) => fs.writeFileSync(path.join(dir, name), data);
  write('native/main.cpp', 'void frame() {}\n');
  write('docs/sdd/index.md', '# System\n');
  write('docs/sdd/facts.json', JSON.stringify({meta: {source_commit: 'old'}, classes: {}}));
  execFileSync('git', ['-c', 'core.autocrlf=false', 'add', '.'], {cwd: dir});
  return {dir, write};
}
test('missing and pending review records fail closed', t => {
  const {dir, write} = fixture(t);
  assert.throws(() => check(dir), /No review/);
  write('docs/sdd-review.json', '{"version":1,"reviewer":null}');
  assert.throws(() => check(dir), /pending/);
});
test('source edits, added files, deleted files, and documentation edits invalidate review', t => {
  const {dir, write} = fixture(t);
  accept(dir, 'Test Reviewer', {}, 'Reviewed fixture');
  assert.equal(check(dir).reviewer, 'Test Reviewer');
  write('native/main.cpp', 'void changed() {}\n');
  assert.throws(() => check(dir), /native\/main.cpp/);
  write('native/main.cpp', 'void frame() {}\n');
  write('native/new.cpp', 'void newGame() {}\n');
  assert.throws(() => check(dir), /native\/new.cpp/);
  fs.unlinkSync(path.join(dir, 'native/new.cpp'));
  write('native/second.cpp', 'void second() {}\n');
  accept(dir, 'Test Reviewer', {}, 'Reviewed added source');
  fs.unlinkSync(path.join(dir, 'native/second.cpp'));
  assert.throws(() => check(dir), /native\/second.cpp/);
  write('native/second.cpp', 'void second() {}\n');
  write('docs/sdd/index.md', '# Unsupported claim\n');
  assert.throws(() => check(dir), /docs\/sdd\/index.md/);
});
test('line endings and volatile extraction metadata do not invalidate review', t => {
  const {dir, write} = fixture(t);
  accept(dir, 'Test Reviewer', {}, 'Reviewed fixture');
  write('native/main.cpp', 'void frame() {}\r\n');
  write('docs/sdd/facts.json', JSON.stringify({classes: {}, meta: {source_commit: 'new', source_root: '/ci'}}));
  assert.doesNotThrow(() => check(dir));
  write('docs/sdd/facts.json', JSON.stringify({classes: {NewClass: {}}, meta: {}}));
  assert.throws(() => check(dir), /facts.json/);
});
test('needs-review requires a page-specific reason and cannot silently survive edits', t => {
  const {dir, write} = fixture(t);
  const page = 'docs/sdd/threading.md';
  write(page, '---\nstatus: needs-review\n---\nStatic analysis limitation\n');
  assert.throws(() => accept(dir, 'Test Reviewer', {}, 'Reviewed fixture'), /acknowledge/);
  accept(dir, 'Test Reviewer', {[page]: 'Runtime timing remains explicitly unverified'}, 'Reviewed fixture');
  assert.doesNotThrow(() => check(dir));
  write(page, '---\nstatus: needs-review\n---\nDifferent assertion\n');
  assert.throws(() => check(dir), /threading.md/);
});
test('build settings and new assets are review inputs', t => {
  const {dir, write} = fixture(t);
  accept(dir, 'Test Reviewer', {}, 'Reviewed fixture');
  write('app/src/main/AndroidManifest.xml', '<manifest/>');
  assert.throws(() => check(dir), /AndroidManifest/);
  assert.ok(snapshot(dir).source['app/src/main/AndroidManifest.xml']);
  assert.deepEqual(normalizedFacts({meta: {generated_at: 'now', errors: 1}}), {meta: {errors: 1}});
});
test('CI cannot manufacture a review record', t => {
  const {dir} = fixture(t);
  assert.throws(() => execFileSync(process.execPath, [path.resolve('tools/docs-review.mjs'), 'accept',
    '--reviewer', 'Bot', '--reason', 'Automatic'], {cwd: dir, env: {...process.env, CI: 'true'}, stdio: 'pipe'}),
    error => error.status === 1 && error.stderr.toString().includes('CI must not'));
});
const fingerprint = 'ab'.repeat(32);
const certificates = `Signer #1 certificate SHA-256 digest: ${fingerprint}\n`;
const badging = "package: name='com.ttolsun.minigameheaven' versionCode='1' versionName='0.1.0'\n";
test('release accepts only the intended package, version and signing certificate', () => {
  assert.equal(validateRelease('v0.1.0', badging, certificates, fingerprint, 'com.ttolsun.minigameheaven').versionCode, 1);
  for (const [tag, apk, cert, expected] of [
    ['v0.2.0', badging, certificates, fingerprint],
    ['v0.1.0', badging.replace('com.ttolsun.minigameheaven', 'wrong.package'), certificates, fingerprint],
    ['v0.1.0', badging + 'application-debuggable\n', certificates, fingerprint],
    ['v0.1.0', badging, certificates, 'cd'.repeat(32)],
    ['v0.1.0', badging, '', fingerprint],
    ['v0.1.0', badging, certificates, undefined],
    ['v0.1.0;echo bad', badging, certificates, fingerprint],
  ]) assert.throws(() => validateRelease(tag, apk, cert, expected, 'com.ttolsun.minigameheaven'));
  assert.equal(validateRelease('v0.1.0', badging.replace('com.ttolsun.minigameheaven', 'com.example.renamed'),
    certificates, fingerprint, 'com.example.renamed').version, '0.1.0');
});
