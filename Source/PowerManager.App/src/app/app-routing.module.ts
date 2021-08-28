import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { AuthorizationFilter } from './common/authorizationFilter.component';

const routes: Routes = [
  {
    path: '',
    canActivate: [AuthorizationFilter],
    pathMatch: 'full',
    loadChildren: () => import('./home/home.module').then(m => m.HomeModule)
  },
  {
    path: 'Home',
    canActivate: [AuthorizationFilter],
    pathMatch: 'full',
    loadChildren: () => import('./home/home.module').then(m => m.HomeModule)
  },
  {
    path: 'Login',
    pathMatch: 'full',
    loadChildren: () => import('./login/login.module').then(m => m.LoginModule)
  }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
