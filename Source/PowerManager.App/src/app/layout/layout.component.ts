import { Component, AfterViewInit, AfterViewChecked, NgModule } from '@angular/core';

declare const $: any;

@Component({
  selector: 'layout',
  templateUrl: './layout.html',
  styleUrls: ['./layout.css']
})
export class LayoutComponent implements AfterViewInit {

  constructor() { }

  ngAfterViewInit() {
    $('#home').addClass('active');
  }
}
